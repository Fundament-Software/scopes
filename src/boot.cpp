/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "boot.hpp"
#include "timer.hpp"
#include "gc.hpp"
#include "error.hpp"
#include "lexerparser.hpp"
#include "source_file.hpp"
#include "prover.hpp"
#include "list.hpp"
#include "execution.hpp"
#include "globals.hpp"
#include "scope.hpp"
#include "expander.hpp"
#include "types.hpp"
#include "gen_llvm.hpp"
#include "compiler_flags.hpp"

#include "scopes/scopes.h"

#ifdef SCOPES_WIN32
#include "stdlib_ex.h"
#include "dlfcn.h"
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <cstdlib>
#include <string.h>

#if SCOPES_USE_WCHAR
#include <codecvt>
#endif

#include "llvm/ExecutionEngine/SectionMemoryManager.h"

namespace scopes {

static Timer *main_compile_time = nullptr;
void on_startup() {
    main_compile_time = new Timer(TIMER_Main);
}

void on_shutdown() {
    delete main_compile_time;
#if SCOPES_PRINT_TIMERS
    //print_profiler_info();
    Timer::print_timers();
    StyledStream ss(SCOPES_CERR);
    ss << "largest recorded stack size: " << g_largest_stack_size << std::endl;
#endif
}

bool signal_abort = false;
void f_abort() {
    on_shutdown();
    if (signal_abort) {
        std::abort();
    } else {
        exit(1);
    }
}


void f_exit(int c) {
    on_shutdown();
    exit(c);
}


SCOPES_RESULT(ValueRef) load_custom_core(const char *executable_path) {
    SCOPES_RESULT_TYPE(ValueRef);
    // attempt to read bootstrap expression from end of binary
    auto file = SourceFile::from_file(
        Symbol(String::from_cstr(executable_path)));
    if (!file) {
        SCOPES_ERROR(MainInaccessibleBinary);
    }
    auto ptr = file->strptr();
    auto size = file->size();
    auto cursor = ptr + size - 1;
    while ((*cursor == '\n')
        || (*cursor == '\r')
        || (*cursor == ' ')) {
        // skip the trailing text formatting garbage
        // that win32 echo produces
        cursor--;
        if (cursor < ptr) return ValueRef();
    }
    if (*cursor != ')') return ValueRef();
    cursor--;
    // seek backwards to find beginning of expression
    while ((cursor >= ptr) && (*cursor != '('))
        cursor--;
    LexerParser footerParser(std::move(file), cursor - ptr);
    auto expr = SCOPES_GET_RESULT(extract_list_constant(SCOPES_GET_RESULT(footerParser.parse())));
    if (expr == EOL) {
        SCOPES_ERROR(InvalidFooter);
    }
    auto it = SCOPES_GET_RESULT(extract_list_constant(expr->at));
    if (it == EOL) {
        SCOPES_ERROR(InvalidFooter);
    }
    auto head = it->at;
    auto sym = SCOPES_GET_RESULT(extract_symbol_constant(head));
    if (sym != Symbol("core-size"))  {
        SCOPES_ERROR(InvalidFooter);
    }
    it = it->next;
    if (it == EOL) {
        SCOPES_ERROR(InvalidFooter);
    }
    auto script_size = SCOPES_GET_RESULT(extract_integer_constant(it->at));
    if (script_size <= 0) {
        SCOPES_ERROR(InvalidFooter);
    }
    LexerParser parser(std::move(file), cursor - script_size - ptr, script_size);
    return parser.parse();
}

//------------------------------------------------------------------------------
// SCOPES CORE
//------------------------------------------------------------------------------

/* this function looks for a header at the end of the compiler executable
   that indicates a scopes core.

   the header has the format (core-size <size>), where size is a i32 value
   holding the size of the core source file in bytes.

   the compiler uses this function to override the default scopes core 'core.sc'
   located in the compiler's directory.

   to later override the default core file and load your own, cat the new core
   file behind the executable and append the header, like this:

   $ cp scopes myscopes
   $ cat mycore.sc >> myscopes
   $ echo "(core-size " >> myscopes
   $ wc -c < mycore.sc >> myscopes
   $ echo ")" >> myscopes

   */


//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

static bool terminal_supports_ansi() {
#ifdef SCOPES_WIN32
    if (isatty(STDOUT_FILENO))
        return true;
    return getenv("TERM") != nullptr;
#else
    //return isatty(fileno(stdout));
    return isatty(STDOUT_FILENO);
#endif
}

static void setup_stdio() {
    if (terminal_supports_ansi()) {
        stream_default_style = stream_ansi_style;
        #ifdef SCOPES_WIN32
        #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
        #endif

        // turn on ANSI code processing
        auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        auto hStdErr = GetStdHandle(STD_ERROR_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdOut, &mode);
        SetConsoleMode(hStdOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        GetConsoleMode(hStdErr, &mode);
        SetConsoleMode(hStdErr, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        setbuf(stdout, 0);
        setbuf(stderr, 0);
#if SCOPES_USE_WCHAR
        _setmode(_fileno(stdout), _O_U16TEXT);
        _setmode(_fileno(stderr), _O_U16TEXT);
        //std::wcout.imbue(std::locale(std::locale("C"), new std::codecvt_utf8<wchar_t>));
        //std::wcerr.imbue(std::locale(std::locale("C"), new std::codecvt_utf8<wchar_t>));
#else
        SetConsoleOutputCP(CP_UTF8);
        _setmode(_fileno(stdout), _O_BINARY);
        _setmode(_fileno(stderr), _O_BINARY);
        //fcntl(_fileno(stdout), F_SETFL, fcntl(_fileno(stdout), F_GETFL) | O_NONBLOCK);
#endif
        #endif
    }
}

void init(void *c_main, int argc, char *argv[]) {
    using namespace scopes;
    on_startup();

    Symbol::_init_symbols();
    init_llvm();

    setup_stdio();
    scopes_argc = argc;
    scopes_argv = argv;

    std::string exepath = llvm::sys::fs::getMainExecutable(argv[0], c_main);

    scopes_compiler_path = nullptr;
    scopes_compiler_dir = nullptr;
    scopes_clang_include_dir = nullptr;
    scopes_include_dir = nullptr;
    if (argv) {
        if (argv[0]) {
            std::string loader = exepath;
            // string must be kept resident
            scopes_compiler_path = strdup(loader.c_str());
        } else {
            scopes_compiler_path = strdup("");
        }

        char *path_copy = strdup(scopes_compiler_path);
        scopes_compiler_dir = format("%s/..", dirname(path_copy))->data;
        free(path_copy);
        scopes_clang_include_dir = format("%s/lib/clang/include", scopes_compiler_dir)->data;
        scopes_include_dir = format("%s/include", scopes_compiler_dir)->data;
    }

    init_types();
    init_globals(argc, argv);
}

SCOPES_RESULT(int) try_main() {
    SCOPES_RESULT_TYPE(int);
    using namespace scopes;

    ValueRef expr = SCOPES_GET_RESULT(load_custom_core(scopes_compiler_path));
    if (expr) {
        goto skip_regular_load;
    }

    {
#if 0
        Symbol name = format("%s/lib/scopes/%i.%i.%i/core.sc",
            scopes_compiler_dir,
            SCOPES_VERSION_MAJOR,
            SCOPES_VERSION_MINOR,
            SCOPES_VERSION_PATCH);
#else
        Symbol name = format("%s/lib/scopes/core.sc",
            scopes_compiler_dir);
#endif
        auto sf = SourceFile::from_file(name);
        if (!sf) {
            SCOPES_ERROR(CoreMissing, name);
        }
        LexerParser parser(std::move(sf));
        expr = SCOPES_GET_RESULT(parser.parse());
    }

skip_regular_load:
    const Anchor *anchor = expr.anchor();
    auto list = SCOPES_GET_RESULT(extract_list_constant(expr));
    TemplateRef tmpfn = SCOPES_GET_RESULT(expand_module(anchor, list, Scope::from(sc_get_globals())));

#if 0 //SCOPES_DEBUG_CODEGEN
    StyledStream ss(std::cout);
    std::cout << "non-normalized:" << std::endl;
    stream_ast(ss, tmpfn, StreamASTFormat());
    std::cout << std::endl;
#endif

    FunctionRef fn = SCOPES_GET_RESULT(prove(FunctionRef(), tmpfn, {}));

    auto main_func_type = native_opaque_pointer_type(raising_function_type(
        arguments_type({}), {}));

    auto stage_func_type = native_opaque_pointer_type(raising_function_type(
        arguments_type({TYPE_CompileStage}), {}));

    const int compile_flags = CF_Cache;

compile_stage:
    if (fn->get_type() == stage_func_type) {
        typedef sc_valueref_raises_t (*StageFuncType)();
        StageFuncType fptr = (StageFuncType)SCOPES_GET_RESULT(compile(fn, compile_flags))->value;
        auto result = fptr();
        if (!result.ok) {
            SCOPES_RETURN_ERROR(result.except);
        }
        auto value = result._0;
        if (value.isa<Function>()) {
            fn = value.cast<Function>();
            goto compile_stage;
        } else {
            return 0;
        }
    }

    if (fn->get_type() != main_func_type) {
        SCOPES_ERROR(CoreModuleFunctionTypeMismatch, fn->get_type(), main_func_type);
    }

#if 0 //SCOPES_DEBUG_CODEGEN
    std::cout << "normalized:" << std::endl;
    stream_ast(ss, fn, StreamASTFormat());
    std::cout << std::endl;

    compile_flags |= CF_DumpModule;
#endif

    typedef sc_void_raises_t (*MainFuncType)();
    MainFuncType fptr = (MainFuncType)SCOPES_GET_RESULT(compile(fn, compile_flags))->value;
    {
        auto result = fptr();
        if (!result.ok) {
            SCOPES_RETURN_ERROR(result.except);
        }
    }

    return 0;
}

#if 0
#ifndef SCOPES_WIN32
static void crash_handler(int sig) {
  void *array[20];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 20);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif
#endif

int run_main() {
    using namespace scopes;
    auto result = try_main();
    if (!result.ok()) {
        print_error(result.assert_error());
        f_exit(1);
    }
    f_exit(result.assert_ok());
    return 0;
}

} // namespace scopes
