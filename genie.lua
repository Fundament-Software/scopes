
local THISDIR = os.getcwd()
local CLANG_PATH
local MSYS_BASE_PATH = "C:/msys64"
if os.is("windows") then
    -- try to find msys/usr/bin
    local testpath = os.pathsearch("msys-2.0.dll", os.getenv("PATH"))
    if testpath then
        MSYS_BASE_PATH = path.getabsolute(testpath .. "/../..")
    end
    print("MSYS_BASE_PATH = " .. MSYS_BASE_PATH)
end
local MINGW_BASE_PATH = MSYS_BASE_PATH .. "/mingw64"
local MSYS_BIN_PATH = MSYS_BASE_PATH .. "/usr/bin"
if os.is("linux") then
    CLANG_PATH = THISDIR .. "/clang/bin:" .. os.getenv("PATH")
elseif os.is("windows") then
    CLANG_PATH = MINGW_BASE_PATH .. "/bin"
elseif os.is("macosx") then
    CLANG_PATH = "/clang/bin:" .. os.outputof("brew --prefix llvm|tr -d '\n'") .. "/bin"
else
    error("unsupported os")
end
local BINDIR = THISDIR .. "/bin"

local USE_ASAN_UBSAN = false
local ASAN_USAN_OPTS = {
    "-fsanitize=address",
    "-fsanitize-address-use-after-scope",
    "-fno-omit-frame-pointer",
    "-fsanitize=undefined",
    "-fno-common",
}

local function flatten(t)
    local result = {}
    local function iterate(t)
        for _,k in pairs(t) do
            if type(k) == "table" then
                iterate(k)
            elseif k ~= nil and k ~= "" then
                table.insert(result, k)
            end
        end
    end
    iterate(t)
    return result
end

local function pkg_config(cmd)
    local args = os.outputof(cmd)
    --print(cmd, "=>", args)
    return flatten(string.explode(args, "[ \n]+"))
end

local function finddir(name, searchpath)
    searchpath = searchpath or CLANG_PATH
    local path = os.pathsearch(name, searchpath)
    assert(path, name .. " not found in path " .. searchpath)
    return path .. "/" .. name
end

local function dllpath(name, searchpath)
    searchpath = searchpath or CLANG_PATH
    assert(os.is("windows"))
    name = name .. ".dll"
    local path = os.pathsearch(name, searchpath)
    assert(path, name .. " not found in path " .. searchpath)
    return path .. "/" .. name
end

local function toolpath(name, searchpath)
    searchpath = searchpath or CLANG_PATH
    if os.is("windows") then
        name = name .. ".exe"
    end
    local path = os.pathsearch(name, searchpath)
    assert(path, name .. " not found in path " .. searchpath)
    return path .. "/" .. name
end

local function print_list(l)
    for k,v in pairs(l) do
        print(k,v)
    end
end

local CLANG_CXX = toolpath("clang++", CLANG_PATH)
local CLANG_CC = toolpath("clang", CLANG_PATH)
local LLVM_CONFIG = toolpath("llvm-config", CLANG_PATH)

local LLVM_LDFLAGS = pkg_config(LLVM_CONFIG .. " --ldflags")
local LLVM_CXXFLAGS = pkg_config(LLVM_CONFIG .. " --cxxflags")
local TARGET_COMPONENTS = ""
local LLVM_LIBS = pkg_config(LLVM_CONFIG .. " --link-static --libs orcjit"
    .. " engine passes option objcarcopts coverage support lto coroutines"
    .. " webassembly frontendopenmp native orcshared orctargetprocess jitlink"
    .. " " .. TARGET_COMPONENTS)
local LLVM_INCLUDEDIR = pkg_config(LLVM_CONFIG .. " --includedir")

local CLANG_DEPS = {
    "-lclangCodeGen",
    "-lclangFrontend",
    "-lclangDriver",
    "-lclangSerialization",
    "-lclangParse",
    "-lclangSema",
    "-lclangAnalysis",
    "-lclangEdit",
    "-lclangASTMatchers",
    "-lclangAST",
    "-lclangLex",
    "-lclangBasic",
}
--local POLLY_DEPS = {
--    "-lPolly",
--    "-lPollyISL",
--}

if not os.is("windows") then
    premake.gcc.cxx = CLANG_CXX
    premake.gcc.cc = CLANG_CC
    premake.gcc.llvm = true
    if USE_ASAN_UBSAN then
        premake.gcc.ld = CLANG_CXX
    end

end

solution "scopes"
    location "build"
    configurations { "debug", "release" }
    platforms { "native", "x64" }

project "gensyms"
    kind "ConsoleApp"
    language "C++"
    files {
        "src/hash.cpp",
        "src/gensyms.cpp",
    }
    targetdir "bin"

    includedirs {
        "external",
    }

    postbuildcommands {
        BINDIR .. "/gensyms > " .. THISDIR .. "/src/known_symbols.hpp"
    }

    configuration { "linux" }
        buildoptions_cpp {
            "-ferror-limit=1",
        }


project "scopesrt"
    kind "SharedLib"
    language "C++"
    files {
        "src/globalsyms.c",
        "src/value.cpp",
        "src/gc.cpp",
        "src/symbol_enum.cpp",
        "src/string.cpp",
        "src/styled_stream.cpp",
        "src/utils.cpp",
        "src/symbol.cpp",
        "src/timer.cpp",
        "src/source_file.cpp",
        "src/anchor.cpp",
        "src/type.cpp",
        "src/builtin.cpp",
        "src/error.cpp",
        "src/boot.cpp",
        "src/type/integer_type.cpp",
        "src/type/real_type.cpp",
        "src/type/pointer_type.cpp",
        "src/type/sized_storage_type.cpp",
        "src/type/array_type.cpp",
        "src/type/vector_type.cpp",
        "src/type/matrix_type.cpp",
        "src/type/tuple_type.cpp",
        "src/type/qualify_type.cpp",
        "src/type/arguments_type.cpp",
        "src/type/function_type.cpp",
        "src/type/typename_type.cpp",
        "src/type/image_type.cpp",
        "src/type/sampledimage_type.cpp",
        "src/qualifier/unique_qualifiers.cpp",
        "src/qualifier/key_qualifier.cpp",
        "src/qualifier/refer_qualifier.cpp",
        "src/scope.cpp",
        "src/list.cpp",
        "src/lexerparser.cpp",
        "src/stream_anchors.cpp",
        "src/stream_expr.cpp",
        "src/c_import.cpp",
        "src/execution.cpp",
        "src/prover.cpp",
        "src/lifetime.cpp",
        "src/quote.cpp",
        "src/platform_abi.cpp",
        "src/gen_spirv.cpp",
        "src/gen_llvm.cpp",
        "src/expander.cpp",
        "src/globals.cpp",
        "src/hash.cpp",
        "src/cache.cpp",
        "external/linenoise-ng/src/linenoise.cpp",
        "external/linenoise-ng/src/ConvertUTF.cpp",
        "external/linenoise-ng/src/wcwidth.cpp",
        "external/glslang/SpvBuilder.cpp",
        "external/glslang/Logger.cpp",
        "external/glslang/InReadableOrder.cpp",
        "external/glslang/disassemble.cpp",
        "external/glslang/doc.cpp",
        --"external/coro/coro.c",
        "SPIRV-Cross/spirv_cross_parsed_ir.cpp",
        "SPIRV-Cross/spirv_parser.cpp",
        "SPIRV-Cross/spirv_glsl.cpp",
        "SPIRV-Cross/spirv_cross.cpp",
        "SPIRV-Cross/spirv_cfg.cpp",
    }
    links {
        "gensyms"
    }
    --custombuildtask {
    --    {
    --        "src/symbol_enum.inc", "src/known_symbols.hpp",
    --        { BINDIR .. "/gensyms", "src/gensyms.cpp", },
    --        { "$(1) $(<) > $(@)" }
    --    }
    --}
    includedirs {
        "external/linenoise-ng/include",
        "external",
        "SPIRV-Tools/include",
        "include",
        "."
    }
    libdirs {
        --"bin",
        --"build/src/nanovg/build",
        --"build/src/tess2/Build",
        --"build/src/stk/src",
        --"build/src/nativefiledialog/src",
    }
    targetdir "bin"
    defines {
        "SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS",
        "SCOPESRT_IMPL"
    }

    configuration { "linux" }
        defines { "SCOPES_LINUX" }

        includedirs {
			LLVM_INCLUDEDIR
        }

        buildoptions_cpp {
            "-std=c++14",
            "-fno-rtti",
            "-fno-exceptions",
            "-ferror-limit=1",
            "-pedantic",
            "-Wall",
            "-Wno-keyword-macro",
            "-Wno-gnu-redeclared-enum",
            "-Werror=switch",
            "-fdiagnostics-absolute-paths"
        }

        if USE_ASAN_UBSAN then
            buildoptions_cpp(ASAN_USAN_OPTS)
            buildoptions_c(ASAN_USAN_OPTS)
            linkoptions(ASAN_USAN_OPTS)
        end

        defines {
            --"_GLIBCXX_USE_CXX11_ABI=0",
        }

        links {
            "pthread", "m", "tinfo", "dl", "z",
        }

        linkoptions {
            --"-Wl,--stack,8388608",
            --"-Wl,--stack,16777216",
            --"-Wl,--stack,67108864", -- 64 GB
            "-Wl,-soname,libscopesrt.so",
            "-Wl,--version-script=" .. THISDIR .. "/src/libscopesrt.map",
        }

        linkoptions {
            --"-Wl,--whole-archive",
            --"-l...",
            --"-Wl,--no-whole-archive",

            -- can't use this or our LLVM will collide with LLVM in other libs
            --"-Wl,--export-dynamic",
            --"-rdynamic",

            THISDIR .. "/SPIRV-Tools/build/source/opt/libSPIRV-Tools-opt.a",
            THISDIR .. "/SPIRV-Tools/build/source/libSPIRV-Tools.a"
        }
        linkoptions(LLVM_LDFLAGS)
        linkoptions(CLANG_DEPS)
        --linkoptions(POLLY_DEPS)
        --linkoptions { "-Wl,--whole-archive" }
        linkoptions(LLVM_LIBS)
        --linkoptions { "-Wl,--no-whole-archive" }

        postbuildcommands {
            -- BINDIR .. "/scopes " .. THISDIR .. "/testing/test_all.sc"
        }

    configuration { "windows" }
        buildoptions_cpp {
            "-D_GNU_SOURCE",
            "-Wa,-mbig-obj",
            "-std=gnu++14",
            "-fno-exceptions",
            "-fno-rtti",
            "-fno-strict-aliasing",
            "-D__STDC_CONSTANT_MACROS",
            "-D__STDC_FORMAT_MACROS",
            "-D__STDC_LIMIT_MACROS",
        }

        buildoptions_cpp {
            "-Wall",
        }

        -- gcc-only options
        buildoptions_cpp {
            "-Wno-error=date-time",
            "-fmax-errors=1",
            "-Wno-nonnull-compare",
            "-Wno-unused-but-set-variable",
            "-Wno-sign-compare",
            "-Wno-vla",
            "-Wno-enum-compare",
            "-Wno-comment",
            "-Wno-misleading-indentation",
            "-Wno-pragmas",
            --"-Wno-return-type",
            "-Wno-variadic-macros",
            "-Wno-int-in-bool-context"
        }

        buildoptions_cpp {
            "-Wno-unused-variable",
            "-Wno-unused-function",
        }

        includedirs {
            "src/win32"
        }

        includedirs {
			LLVM_INCLUDEDIR
        }

        files {
            "src/win32/mman.c",
            "src/win32/realpath.c",
            "src/win32/dlfcn.c",
        }

        defines {
            "SCOPES_WIN32",
        }

        buildoptions_c {
            "-Wno-shift-count-overflow"
        }

        links {
            "uuid", "ole32", "psapi", "version", "stdc++", "z", "mingwex"
        }

        linkoptions {
            "-Wl,--exclude-all-symbols",
            "-Wl,--stack,8388608"
        }
        linkoptions {
            THISDIR .. "/SPIRV-Tools/build/source/opt/libSPIRV-Tools-opt.a",
            THISDIR .. "/SPIRV-Tools/build/source/libSPIRV-Tools.a"
        }
        linkoptions(LLVM_LDFLAGS)
        linkoptions(CLANG_DEPS)
        linkoptions(LLVM_LIBS)

        if os.is("windows") then
            local CP = toolpath("cp", MSYS_BIN_PATH)

            postbuildcommands {
                CP .. " -v " .. dllpath("libgcc_s_seh-1") .. " " .. BINDIR,
                CP .. " -v " .. dllpath("libstdc++-6") .. " " .. BINDIR,
                CP .. " -v " .. dllpath("libwinpthread-1") .. " " .. BINDIR,
                CP .. " -v " .. dllpath("zlib1") .. " " .. BINDIR,
            }
        end

        postbuildcommands {
            -- BINDIR .. "/scopes " .. THISDIR .. "/testing/test_all.sc"
        }

    configuration { "macosx" }
        defines { "SCOPES_MACOS" }

        includedirs {
			LLVM_INCLUDEDIR
        }

        buildoptions_cpp {
            "-std=c++14",
            "-fno-rtti",
            "-fno-exceptions",
            "-ferror-limit=1",
            "-pedantic",
            "-Wall",
            "-Wno-keyword-macro",
            "-Wno-gnu-redeclared-enum",
        }

        if USE_ASAN_UBSAN then
            buildoptions_cpp(ASAN_USAN_OPTS)
            buildoptions_c(ASAN_USAN_OPTS)
            linkoptions(ASAN_USAN_OPTS)
        end

        links {
            "pthread", "m", "ncurses", "dl", "z",
        }

        linkoptions {
            THISDIR .. "/SPIRV-Tools/build/source/opt/libSPIRV-Tools-opt.a",
            THISDIR .. "/SPIRV-Tools/build/source/libSPIRV-Tools.a"
        }

        linkoptions(LLVM_LDFLAGS)
        linkoptions(CLANG_DEPS)
        --linkoptions(POLLY_DEPS)
        linkoptions(LLVM_LIBS)

        postbuildcommands {
            -- BINDIR .. "/scopes " .. THISDIR .. "/testing/test_all.sc"
        }

    configuration "debug"
        defines { "SCOPES_DEBUG" }
        flags { "Symbols" }

        buildoptions_cpp {
            "-O0"
        }

    configuration "release"
        --defines { "NDEBUG" }
        flags { "Optimize", "Symbols" }

project "scopes"
    kind "ConsoleApp"
    language "C++"
    files {
        "src/main.cpp",
    }
    includedirs {
        "include",
    }
    links {
        "scopesrt",
    }
    targetdir "bin"
    configuration { "linux" }
        defines { "SCOPES_LINUX" }

        includedirs {
            --"clang/include"
        }

        buildoptions_cpp {
            "-std=c++14",
            "-fno-rtti",
            "-fno-exceptions",
            "-ferror-limit=1",
            "-pedantic",
            "-Wall",
            "-Wno-keyword-macro",
            "-Wno-gnu-redeclared-enum",
            "-fdiagnostics-absolute-paths"
        }

        defines {
            --"_GLIBCXX_USE_CXX11_ABI=0",
        }

        links {
            --"dl",
            --"pthread",
            --"ncurses",
        }

        linkoptions {
            --"-Wl,--stack,8388608"
            --"-Wl,--stack,16777216"
            "-Wl,-rpath=\\$$ORIGIN"
        }

        if USE_ASAN_UBSAN then
            buildoptions_cpp(ASAN_USAN_OPTS)
            buildoptions_c(ASAN_USAN_OPTS)
            linkoptions(ASAN_USAN_OPTS)
        end

        postbuildcommands {
        }

    configuration { "windows" }
        buildoptions_cpp {
            "-D_GNU_SOURCE",
            "-Wa,-mbig-obj",
            "-std=gnu++14",
            "-fno-exceptions",
            "-fno-rtti",
            "-fno-strict-aliasing",
            "-D__STDC_CONSTANT_MACROS",
            "-D__STDC_FORMAT_MACROS",
            "-D__STDC_LIMIT_MACROS",
        }

        buildoptions_cpp {
            "-Wall",
        }

        -- gcc-only options
        buildoptions_cpp {
            "-Wno-error=date-time",
            "-fmax-errors=1",
            "-Wno-vla",
            "-Wno-enum-compare",
            "-Wno-comment",
            "-Wno-misleading-indentation",
            "-Wno-pragmas",
            --"-Wno-return-type",
            "-Wno-variadic-macros",
            "-Wno-int-in-bool-context"
        }

        buildoptions_cpp {
            "-Wno-unused-variable",
            "-Wno-unused-function",
        }

        includedirs {
            "src/win32",
        }

        files {
            --"src/win32/mman.c",
            --"src/win32/realpath.c",
            --"src/win32/dlfcn.c",
        }

        defines {
            "SCOPES_WIN32",
        }

        buildoptions_c {
            "-Wno-shift-count-overflow"
        }

        links {
            --"uuid",
            --"ole32",
        }

        linkoptions {
            "-Wl,--stack,8388608"
        }

        postbuildcommands {
        }

    configuration { "macosx" }
        defines { "SCOPES_MACOS" }

        buildoptions_cpp {
            "-std=c++14",
            "-fno-rtti",
            "-fno-exceptions",
            "-ferror-limit=1",
            "-pedantic",
            "-Wall",
            "-Wno-keyword-macro",
            "-Wno-gnu-redeclared-enum",
        }

        postbuildcommands {
            "install_name_tool -change ../bin/libscopesrt.dylib @executable_path/libscopesrt.dylib " .. BINDIR .. "/scopes"
        }

    configuration "debug"
        defines { "SCOPES_DEBUG" }
        flags { "Symbols" }

        buildoptions_cpp {
            "-O0"
        }

    configuration "release"
        --defines { "NDEBUG" }
        flags { "Optimize", "Symbols" }
