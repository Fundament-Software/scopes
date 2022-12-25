cmake_minimum_required(VERSION 3.15)
project(scopes-lib VERSION 0.19.0)
option(USE_ASAN_UBSAN "Use address sanitizer" OFF)
option(USE_DEFAULT_FOLDERS "Don't override the cmake output folders with a unified /bin/ folder. Also disables debug postfix." OFF)
option(TARGET_WEBASSEMBLY "Enable WebAssembly target" OFF)
option(TARGET_AARCH64 "Enable AArch64 target" OFF)
option(TARGET_RISCV "Enable RISC-V target" OFF)
option(USE_ASAN_UBSAN "Use address sanitizer" OFF)
option(USE_MIMALLOC "Replace the default allocator with mimalloc" OFF)

if(MSVC)
  # This ensures that we default to static but let vcpkg configure things the way it wants
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<STREQUAL:${VCPKG_CRT_LINKAGE},dynamic>:DLL>" CACHE STRING "")
  set(CPP_WARNINGS "-Wno-error=date-time" "-Wno-unused-but-set-variable" "-Wno-sign-compare" "-Wno-vla" "-Wno-enum-compare" "-Wno-comment" "-Wno-misleading-indentation" "-Wno-pragmas" "-Wno-variadic-macros" "-Wno-int-in-bool-context" "-Wno-unused-variable" "-Wno-unused-function" "-Wno-unused-parameter" "-Wreturn-type-c-linkage")
else()
  set(CPP_WARNINGS "-Wall" "-Wno-keyword-macro" "-Wno-gnu-redeclared-enum" "-Werror=switch")
endif()
set(BIN_DIR "bin")

set(ASAN_USAN_OPTS "-fsanitize=address" "-fsanitize-address-use-after-scope" "-fno-omit-frame-pointer" "-fsanitize=undefined" "-fno-common")

if(MSVC)
  string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
  string(REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
