cmake_minimum_required(VERSION 3.20)

project(WCDBRepairTool VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Prefer static runtime on Windows to match WCDB's prebuilt libs.
# NOTE: Do NOT use CMAKE_MSVC_RUNTIME_LIBRARY here because WCDB enables ASM_MASM,
# and CMake may try to apply MSVC runtime settings to ASM which will fail.
if(MSVC)
  foreach(lang C CXX)
    set(CMAKE_${lang}_FLAGS_DEBUG "${CMAKE_${lang}_FLAGS_DEBUG} /MTd")
    set(CMAKE_${lang}_FLAGS_RELEASE "${CMAKE_${lang}_FLAGS_RELEASE} /MT")
    set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO "${CMAKE_${lang}_FLAGS_RELWITHDEBINFO} /MT")
    set(CMAKE_${lang}_FLAGS_MINSIZEREL "${CMAKE_${lang}_FLAGS_MINSIZEREL} /MT")
  endforeach()
endif()

# ---- WCDB (FetchContent) ----
set(SKIP_WCONAN ON CACHE BOOL "Skip wconan" FORCE)
set(WCDB_CPP ON CACHE BOOL "Build WCDB C++ interface" FORCE)
set(WCDB_BRIDGE OFF CACHE BOOL "Disable bridge" FORCE)
set(WCDB_ZSTD ON CACHE BOOL "Build WCDB with zstd" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)
set(TARGET_NAME wcdb CACHE STRING "WCDB target name" FORCE)

FetchContent_Declare(
  wcdb
  GIT_REPOSITORY https://github.com/Tencent/wcdb.git
  GIT_TAG v2.1.11
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(wcdb)

# WCDB's CMake entry is under src/
add_subdirectory("${wcdb_SOURCE_DIR}/src" "${wcdb_BINARY_DIR}/wcdb-src")

# ---- CLI Tool ----
add_executable(wcdb-repair
  src/main.cpp
)
target_link_libraries(wcdb-repair PRIVATE wcdb)

if(WIN32)
  target_compile_definitions(wcdb-repair PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

