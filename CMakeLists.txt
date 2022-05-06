cmake_minimum_required(VERSION 3.15)
project(scopes-lib VERSION 0.1.10)
option(USE_ASAN_UBSAN "Use address sanitizer" OFF)
option(USE_DEFAULT_FOLDERS "Don't override the cmake output folders with a unified /bin/ folder. Also disables debug postfix." OFF)

if(MSVC)
  # This ensures that we default to static but let vcpkg configure things the way it wants
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<STREQUAL:${VCPKG_CRT_LINKAGE},dynamic>:DLL>" CACHE STRING "")
  set(CPP_WARNINGS "-Wno-error=date-time -Wno-unused-but-set-variable -Wno-sign-compare -Wno-vla -Wno-enum-compare -Wno-comment -Wno-misleading-indentation -Wno-pragmas -Wno-variadic-macros -Wno-int-in-bool-context -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter")
else()
  set(CPP_WARNINGS "-Wno-error=date-time -Wno-nonnull-compare -Wno-unused-but-set-variable -Wno-sign-compare -Wno-vla -Wno-enum-compare -Wno-comment -Wno-misleading-indentation -Wno-pragmas -Wno-variadic-macros -Wno-int-in-bool-context -Wno-unused-variable -Wno-unused-function")
endif()
set(BIN_DIR "bin")

set(ASAN_USAN_OPTS "-fsanitize=address" "-fsanitize-address-use-after-scope" "-fno-omit-frame-pointer" "-fsanitize=undefined" "-fno-common")

add_subdirectory(src)
