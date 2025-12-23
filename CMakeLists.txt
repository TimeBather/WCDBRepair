cmake_minimum_required(VERSION 3.20)

project(WCDBRepairTool VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Prefer static runtime on Windows to match WCDB's prebuilt libs.
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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

