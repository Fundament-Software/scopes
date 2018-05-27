
local THISDIR = os.getcwd()
local CLANG_PATH
local MSYS_BASE_PATH = "C:/msys64"
local MINGW_BASE_PATH = MSYS_BASE_PATH .. "/mingw64"
local MSYS_BIN_PATH = MSYS_BASE_PATH .. "/usr/bin"
if os.is("linux") then
    CLANG_PATH = THISDIR .. "/clang/bin:/usr/local/bin:/usr/bin"
elseif os.is("windows") then
    CLANG_PATH = MINGW_BASE_PATH .. "/bin"
elseif os.is("macosx") then
    CLANG_PATH = THISDIR .. "/clang/bin:/usr/local/opt/llvm/bin:/usr/local/bin:/usr/bin"
else
    error("unsupported os")
end
local BINDIR = THISDIR .. "/bin"

local USE_ASAN_UBSAN = false

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
local LLVM_LIBS = pkg_config(LLVM_CONFIG .. " --link-static --libs engine passes option objcarcopts coverage support lto coroutines")

if not os.is("windows") then
    premake.gcc.cxx = CLANG_CXX
    premake.gcc.cc = CLANG_CC
    premake.gcc.llvm = true
end

solution "scopes"
    location "build"
    configurations { "debug", "release" }
    platforms { "native", "x64" }

project "scopes"
    kind "ConsoleApp"
    language "C++"
    files {
        "src/globalsyms.c",
        "src/gc.cpp",
        "src/symbol_enum.cpp",
        "src/none.cpp",
        "src/string.cpp",
        "src/styled_stream.cpp",
        "src/utils.cpp",
        "src/symbol.cpp",
        "src/timer.cpp",
        "src/source_file.cpp",
        "src/anchor.cpp",
        "src/type.cpp",
        "src/builtin.cpp",
        "src/any.cpp",
        "src/error.cpp",
        "src/integer.cpp",
        "src/real.cpp",
        "src/pointer.cpp",
        "src/sized_storage.cpp",
        "src/array.cpp",
        "src/vector.cpp",
        "src/argument.cpp",
        "src/tuple.cpp",
        "src/union.cpp",
        "src/extern.cpp",
        "src/return.cpp",
        "src/function.cpp",
        "src/typename.cpp",
        "src/typefactory.cpp",
        "src/image.cpp",
        "src/sampledimage.cpp",
        "src/main.cpp",
        "src/scope.cpp",
        "src/list.cpp",
        "src/syntax.cpp",
        "src/lexerparser.cpp",
        "src/stream_anchors.cpp",
        "src/stream_expr.cpp",
        "src/parameter.cpp",
        "src/body.cpp",
        "src/label.cpp",
        "src/closure.cpp",
        "src/frame.cpp",
        "src/stream_label.cpp",
        "src/stream_frame.cpp",
        "src/c_import.cpp",
        "src/execution.cpp",
        "src/specializer.cpp",
        "src/platform_abi.cpp",
        "src/scc.cpp",
        "src/gen_spirv.cpp",
        "src/gen_llvm.cpp",
        "src/expander.cpp",
        "src/globals.cpp",
        "src/hash.cpp",
        "external/cityhash/city.cpp",
        "external/linenoise-ng/src/linenoise.cpp",
        "external/linenoise-ng/src/ConvertUTF.cpp",
        "external/linenoise-ng/src/wcwidth.cpp",
        "external/glslang/SpvBuilder.cpp",
        "external/glslang/Logger.cpp",
        "external/glslang/InReadableOrder.cpp",
        "external/glslang/disassemble.cpp",
        "external/glslang/doc.cpp",
        "SPIRV-Cross/spirv_glsl.cpp",
        "SPIRV-Cross/spirv_cross.cpp",
        "SPIRV-Cross/spirv_cfg.cpp",
    }
    includedirs {
        "external/linenoise-ng/include",
        "external",
        "libffi/include",
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
        "SCOPES_CPP_IMPL",
        "SCOPES_MAIN_CPP_IMPL",
        "SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS",
    }

    configuration { "linux" }
        defines { "SCOPES_LINUX" }

        includedirs {
            "clang/include"
        }

        buildoptions_cpp {
            "-std=c++11",
            "-fno-rtti",
            "-fno-exceptions",
            "-ferror-limit=1",
            "-pedantic",
            "-Wall",
            "-Wno-keyword-macro",
            "-Wno-gnu-redeclared-enum"
        }

        if USE_ASAN_UBSAN then
            local opts = {
                "-fsanitize=address",
                "-fsanitize-address-use-after-scope",
                "-fno-omit-frame-pointer",
                "-fsanitize=undefined",
                "-fno-common",
            }
            buildoptions_cpp(opts)
            buildoptions_c(opts)
            linkoptions(opts)
        end

        defines {
            --"_GLIBCXX_USE_CXX11_ABI=0",
        }

        links {
            "pthread", "m", "tinfo", "dl", "z",
        }

        linkoptions {
            --"-Wl,--stack,8388608"
            --"-Wl,--stack,16777216"
        }
        linkoptions {
            --"-Wl,--whole-archive",
            --"-l...",
            --"-Wl,--no-whole-archive",

            -- can't use this or our LLVM will collide with LLVM in other libs
            --"-Wl,--export-dynamic",
            --"-rdynamic",

            THISDIR .. "/libffi/.libs/libffi.a",
            THISDIR .. "/SPIRV-Tools/build/source/opt/libSPIRV-Tools-opt.a",
            THISDIR .. "/SPIRV-Tools/build/source/libSPIRV-Tools.a"
        }
        linkoptions(LLVM_LDFLAGS)
        linkoptions {
            "-lclangFrontend",
            "-lclangDriver",
            "-lclangSerialization",
            "-lclangCodeGen",
            "-lclangParse",
            "-lclangSema",
            "-lclangAnalysis",
            "-lclangEdit",
            "-lclangAST",
            "-lclangLex",
            "-lclangBasic"
        }
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
            "-std=gnu++11",
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
            "-Wno-return-type",
            "-Wno-variadic-macros",
            "-Wno-int-in-bool-context"
        }

        buildoptions_cpp {
            "-Wno-unused-variable",
            "-Wno-unused-function",
        }

        includedirs {
            "src/win32",
            MINGW_BASE_PATH .. "/lib/libffi-3.2.1/include"
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
            "ffi", "uuid", "ole32", "psapi", "version", "stdc++",
        }

        linkoptions {
            "-Wl,--stack,8388608"
        }
        linkoptions {
            THISDIR .. "/SPIRV-Tools/build/source/opt/libSPIRV-Tools-opt.a",
            THISDIR .. "/SPIRV-Tools/build/source/libSPIRV-Tools.a"
        }
        linkoptions(LLVM_LDFLAGS)
        linkoptions {
            "-lclangFrontend",
            "-lclangDriver",
            "-lclangSerialization",
            "-lclangCodeGen",
            "-lclangParse",
            "-lclangSema",
            "-lclangAnalysis",
            "-lclangEdit",
            "-lclangAST",
            "-lclangLex",
            "-lclangBasic"
        }
        linkoptions(LLVM_LIBS)

        if os.is("windows") then
            local CP = toolpath("cp", MSYS_BIN_PATH)

            postbuildcommands {
                CP .. " -v " .. dllpath("libffi-6") .. " " .. BINDIR,
                CP .. " -v " .. dllpath("libgcc_s_seh-1") .. " " .. BINDIR,
                CP .. " -v " .. dllpath("libstdc++-6") .. " " .. BINDIR,
                CP .. " -v " .. dllpath("libwinpthread-1") .. " " .. BINDIR,
            }
        end

        postbuildcommands {
            -- BINDIR .. "/scopes " .. THISDIR .. "/testing/test_all.sc"
        }

    configuration { "macosx" }
        defines { "SCOPES_MACOS" }

        includedirs {
            "clang/include",
            "/usr/local/opt/llvm/include"
        }

        buildoptions_cpp {
            "-std=c++11",
            "-fno-rtti",
            "-fno-exceptions",
            "-ferror-limit=1",
            "-pedantic",
            "-Wall",
            "-Wno-keyword-macro",
            "-Wno-gnu-redeclared-enum",
        }

        if USE_ASAN_UBSAN then
            local opts = {
                "-fsanitize=address",
                "-fsanitize-address-use-after-scope",
                "-fno-omit-frame-pointer",
                "-fsanitize=undefined",
                "-fno-common",
            }
            buildoptions_cpp(opts)
            buildoptions_c(opts)
            linkoptions(opts)
        end

        links {
            "pthread", "m", "ncurses", "dl", "z",
        }

        linkoptions {
            THISDIR .. "/libffi/lib/libffi.a",
            THISDIR .. "/SPIRV-Tools/build/source/opt/libSPIRV-Tools-opt.a",
            THISDIR .. "/SPIRV-Tools/build/source/libSPIRV-Tools.a"
        }

        linkoptions(LLVM_LDFLAGS)

        linkoptions {
            "-lclangFrontend",
            "-lclangDriver",
            "-lclangSerialization",
            "-lclangCodeGen",
            "-lclangParse",
            "-lclangSema",
            "-lclangAnalysis",
            "-lclangEdit",
            "-lclangAST",
            "-lclangLex",
            "-lclangBasic"
        }

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
        defines { "NDEBUG" }
        flags { "Optimize" }


