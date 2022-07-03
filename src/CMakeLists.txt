cmake_minimum_required(VERSION 3.15)

function(retarget_output target)
  if(NOT USE_DEFAULT_FOLDERS)
    set_target_properties(${target} PROPERTIES DEBUG_POSTFIX "-d")
    set_target_properties(${target}
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/${BIN_DIR}"
        PREFIX ""
    )
  endif()
endfunction()

project(gensyms LANGUAGES CXX VERSION ${PROJECT_VERSION})

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

list(APPEND gensyms_SOURCES
    hash.cpp
    gensyms.cpp
)

add_executable(gensyms ${gensyms_SOURCES})
target_include_directories(gensyms PUBLIC ${CMAKE_SOURCE_DIR}/external)
retarget_output(gensyms)

if(MSVC)
  # Cmake does not support linking with a static library
  set_target_properties(gensyms PROPERTIES COMPILE_PDB_NAME "$(TargetName)")
  #set_target_properties(gensyms PROPERTIES VS_PLATFORM_TOOLSET ClangCL)
  target_compile_definitions(gensyms PRIVATE "_HAS_EXCEPTIONS=0")
endif()

# ---------------------------------

project(scopesrt LANGUAGES CXX VERSION ${PROJECT_VERSION})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckSymbolExists)
include(CMakePushCheckState)
include(CheckLibraryExists)

add_custom_command(
  OUTPUT "${CMAKE_SOURCE_DIR}/src/known_symbols.hpp"
  DEPENDS
    gensyms
  COMMAND gensyms ">" "${CMAKE_SOURCE_DIR}/src/known_symbols.hpp"
  VERBATIM
)

find_package(LLVM REQUIRED CONFIG HINTS ${LLVM_HINTS})
find_package(Clang REQUIRED)
find_package(absl CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
add_definitions(${LLVM_DEFINITIONS})

if(MSVC)
  file(GLOB_RECURSE scopesrt_SOURCES "./*.h" "./*.hpp")
endif()

list(APPEND scopesrt_SOURCES
    "globalsyms.c"
    "value.cpp"
    "gc.cpp"
    "symbol_enum.cpp"
    "string.cpp"
    "styled_stream.cpp"
    "utils.cpp"
    "symbol.cpp"
    "timer.cpp"
    "source_file.cpp"
    "anchor.cpp"
    "type.cpp"
    "builtin.cpp"
    "error.cpp"
    "boot.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/integer_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/real_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/pointer_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/sized_storage_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/array_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/vector_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/matrix_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/tuple_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/qualify_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/arguments_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/function_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/typename_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/image_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/type/sampledimage_type.cpp"
    "${CMAKE_SOURCE_DIR}/src/qualifier/unique_qualifiers.cpp"
    "${CMAKE_SOURCE_DIR}/src/qualifier/key_qualifier.cpp"
    "${CMAKE_SOURCE_DIR}/src/qualifier/refer_qualifier.cpp"
    "scope.cpp"
    "list.cpp"
    "lexerparser.cpp"
    "stream_anchors.cpp"
    "stream_expr.cpp"
    "c_import.cpp"
    "execution.cpp"
    "prover.cpp"
    "lifetime.cpp"
    "quote.cpp"
    "platform_abi.cpp"
    "gen_spirv.cpp"
    "gen_llvm.cpp"
    "expander.cpp"
    "globals.cpp"
    "hash.cpp"
    "cache.cpp"
    "${CMAKE_SOURCE_DIR}/external/linenoise-ng/src/linenoise.cpp"
    "${CMAKE_SOURCE_DIR}/external/linenoise-ng/src/ConvertUTF.cpp"
    "${CMAKE_SOURCE_DIR}/external/linenoise-ng/src/wcwidth.cpp"
    "${CMAKE_SOURCE_DIR}/external/glslang/SpvBuilder.cpp"
    "${CMAKE_SOURCE_DIR}/external/glslang/Logger.cpp"
    "${CMAKE_SOURCE_DIR}/external/glslang/InReadableOrder.cpp"
    "${CMAKE_SOURCE_DIR}/external/glslang/disassemble.cpp"
    "${CMAKE_SOURCE_DIR}/external/glslang/doc.cpp"
    #"${CMAKE_SOURCE_DIR}/external/coro/coro.c"
    #"${CMAKE_SOURCE_DIR}/SPIRV-Cross/spirv_cross_parsed_ir.cpp"
    #"${CMAKE_SOURCE_DIR}/SPIRV-Cross/spirv_parser.cpp"
    #"${CMAKE_SOURCE_DIR}/SPIRV-Cross/spirv_glsl.cpp"
    #"${CMAKE_SOURCE_DIR}/SPIRV-Cross/spirv_cross.cpp"
    #"${CMAKE_SOURCE_DIR}/SPIRV-Cross/spirv_cfg.cpp"
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")  
  list(APPEND scopesrt_SOURCES
    "win32/mman.c"
    "win32/realpath.c"
    "win32/dirname.c"
    "win32/basename.c"
    "win32/dlfcn.c"
  )
endif()

add_library(scopesrt ${scopesrt_SOURCES} "${CMAKE_SOURCE_DIR}/src/known_symbols.hpp")
target_include_directories(scopesrt PRIVATE ${LLVM_INCLUDE_DIRS})
#target_include_directories(scopesrt PRIVATE "${LLVM_BUILD_MAIN_SRC_DIR}/../polly/include")
target_include_directories(scopesrt PRIVATE ${CMAKE_SOURCE_DIR}/external/linenoise-ng/include)
target_include_directories(scopesrt PRIVATE ${CMAKE_SOURCE_DIR}/external)
target_include_directories(scopesrt PUBLIC ${CMAKE_SOURCE_DIR}/include)
#target_include_directories(scopesrt PRIVATE ${SPIRV_CROSS_INCLUDE_DIRS})
retarget_output(scopesrt)

if(MSVC)
target_compile_options(scopesrt PRIVATE ${CPP_WARNINGS})
target_compile_definitions(scopesrt PUBLIC "$<IF:$<CONFIG:DEBUG>,DEBUG,NDEBUG>")
else()
target_compile_options(scopesrt PRIVATE "-pedantic" "-fno-exceptions" "-fno-rtti" ${CPP_WARNINGS} "$<$<CONFIG:Debug>:-O0>" "$<$<NOT:$<CONFIG:Debug>>:-O3>")
target_compile_definitions(scopesrt PUBLIC "$<IF:$<CONFIG:DEBUG>,DEBUG,NDEBUG>")
endif()

# Find the libraries that correspond to the LLVM components that we wish to use
#llvm_map_components_to_libnames(llvm_libs orcjit engine passes option objcarcopts coverage support lto coroutines frontendopenmp native WebAssembly X86)
llvm_map_components_to_libnames(llvm_libs orcjit passes option objcarcopts coverage support lto coroutines frontendopenmp native WebAssembly X86)
    
set(clang_libs
    clangCodeGen
    clangFrontend
    clangDriver
    clangSerialization
    clangParse
    clangSema
    clangAnalysis
    clangEdit
    clangASTMatchers
    clangAST
    clangLex
    clangBasic)
      
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  # Check if zlib we found is usable; for example, we may have found a 32-bit
  # library on a 64-bit system which would result in a link-time failure.
  cmake_push_check_state()
  set(CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES ${ZLIB_LIBRARY})
  check_symbol_exists(compress2 zlib.h HAVE_ZLIB)
  cmake_pop_check_state()
endif()

find_package(ZLIB REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_util CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)
find_package(SPIRV-Tools CONFIG REQUIRED)
find_package(SPIRV-Tools-opt CONFIG REQUIRED)
find_package(SPIRV-Tools-link CONFIG REQUIRED)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  find_package(mimalloc CONFIG REQUIRED)
  
  target_compile_definitions(scopesrt PRIVATE SCOPES_WIN32)
  target_include_directories(scopesrt PRIVATE ${CMAKE_SOURCE_DIR}/src/win32)
else()
  find_package(Threads REQUIRED)
  find_library(TINFO_LIBRARY REQUIRED)

  CHECK_LIBRARY_EXISTS(m sin "" HAVE_LIB_M)

  if (HAVE_LIB_M) 
      set(EXTRA_LIBS ${EXTRA_LIBS} m)
  endif (HAVE_LIB_M)

  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_compile_definitions(scopesrt PRIVATE SCOPES_LINUX)
    target_link_options(scopesrt PRIVATE "-Wl,-soname,libscopesrt.so" "-Wl,--version-script=${CMAKE_SOURCE_DIR}/src/libscopesrt.map")
  elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_compile_definitions(scopesrt PRIVATE SCOPES_MACOS)
  else()
    message(FATAL_ERROR, "Unsupported platform ${CMAKE_SYSTEM_NAME}")
  endif()
endif()

if(USE_ASAN_UBSAN)
  target_compile_options(scopesrt PRIVATE ${ASAN_USAN_OPTS})
  target_link_options(scopesrt PRIVATE ${ASAN_USAN_OPTS})
endif()

if(MSVC)
  set_target_properties(scopesrt PROPERTIES VS_PLATFORM_TOOLSET ClangCL)
  set_target_properties(scopesrt PROPERTIES COMPILE_PDB_NAME "$(TargetName)")
  target_compile_definitions(scopesrt PRIVATE "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING")
endif()

option(SCOPES_TARGET_WEBASSEMBLY "Enable WebAssembly target" OFF)
option(SCOPES_TARGET_AARCH64 "Enable AArch64 target" OFF)
option(SCOPES_TARGET_RISCV "Enable RISC-V target" OFF)

if(TARGET_WEBASSEMBLY)
  target_compile_definitions(scopesrt PRIVATE SCOPES_TARGET_WEBASSEMBLY)
endif()
if(TARGET_AARCH64)
  target_compile_definitions(scopesrt PRIVATE SCOPES_TARGET_AARCH64)
endif()
if(TARGET_RISCV)
  target_compile_definitions(scopesrt PRIVATE SCOPES_TARGET_RISCV)
endif()

add_dependencies(scopesrt gensyms)
target_compile_definitions(scopesrt PRIVATE SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS SCOPESRT_IMPL)

target_link_directories(scopesrt PRIVATE ${LLVM_LIBRARY_DIR})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(scopesrt ZLIB::ZLIB ${llvm_libs} spirv-cross-c spirv-cross-cpp spirv-cross-core spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util SPIRV-Tools-static SPIRV-Tools-opt SPIRV-Tools-link ${clang_libs} absl::flat_hash_map absl::flat_hash_set mimalloc-static)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(scopesrt ZLIB::ZLIB ${Threads} ${CMAKE_DL_LIBS} ${TINFO_LIBRARY} ${EXTRA_LIBS} ${llvm_libs} spirv-cross-c spirv-cross-cpp spirv-cross-core spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util SPIRV-Tools-static SPIRV-Tools-opt SPIRV-Tools-link ${clang_libs} absl::flat_hash_map absl::flat_hash_set)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  target_link_libraries(scopesrt ZLIB::ZLIB ${Threads} ${CMAKE_DL_LIBS} ${TINFO_LIBRARY} ${EXTRA_LIBS} ${llvm_libs} spirv-cross-c spirv-cross-cpp spirv-cross-core spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util SPIRV-Tools-static SPIRV-Tools-opt SPIRV-Tools-link ${clang_libs} absl::flat_hash_map absl::flat_hash_set)
endif()

# ---------------------------------

project(scopes LANGUAGES CXX VERSION ${PROJECT_VERSION})

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

list(APPEND scopes_SOURCES
    main.cpp
)

add_executable(scopes ${scopes_SOURCES})
target_link_libraries(scopes scopesrt)
target_include_directories(scopes PRIVATE ${CMAKE_SOURCE_DIR}/external)
retarget_output(scopes)

if(MSVC)
  #set_target_properties(scopes PROPERTIES VS_PLATFORM_TOOLSET ClangCL)
  set_target_properties(scopes PROPERTIES COMPILE_PDB_NAME "$(TargetName)")
  target_compile_definitions(gensyms PRIVATE "_HAS_EXCEPTIONS=0")
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_compile_definitions(scopes PRIVATE SCOPES_WIN32)
  target_include_directories(scopes PRIVATE ${CMAKE_SOURCE_DIR}/src/win32)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_compile_definitions(scopes PRIVATE SCOPES_LINUX)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  target_compile_definitions(scopes PRIVATE SCOPES_MACOS)
else()
  message(FATAL_ERROR, "Unsupported platform ${CMAKE_SYSTEM_NAME}")
endif()

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

install(TARGETS scopesrt scopes
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}  )

install(DIRECTORY "${CMAKE_SOURCE_DIR}/lib/scopes" DESTINATION ${INSTALL_LIB_DIR})

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/scopes" DESTINATION "${INSTALL_INC_DIR}")