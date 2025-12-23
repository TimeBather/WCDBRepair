cmake_minimum_required(VERSION 3.20)

# Workaround for WCDB enabling ASM_MASM on Windows:
# With CMP0091 NEW, CMake may try to apply MSVC runtime library selection
# to ASM targets, which fails at generate time. Set the policy before the
# first project() to make it effective.
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 OLD)
endif()
# Also clear the cache variable to avoid accidental initialization.
set(CMAKE_MSVC_RUNTIME_LIBRARY "" CACHE STRING "Do not use MSVC_RUNTIME_LIBRARY abstraction" FORCE)

project(WCDBRepairTool VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ---- WCDB (FetchContent) ----
set(SKIP_WCONAN ON CACHE BOOL "Skip wconan" FORCE)
set(WCDB_CPP ON CACHE BOOL "Build WCDB C++ interface" FORCE)
set(WCDB_BRIDGE OFF CACHE BOOL "Disable bridge" FORCE)
# zstd 在 Windows/MSVC 下会启用 .S 汇编实现，容易在 CI 环境里失败（缺对象文件/LNK1181）。
# 修复工具本身不依赖 zstd，可直接关闭以提升构建稳定性。
set(WCDB_ZSTD OFF CACHE BOOL "Build WCDB with zstd" FORCE)
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

