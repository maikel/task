# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
cmake_minimum_required(VERSION 3.28...3.31)

project(beman_task VERSION 1.0 LANGUAGES CXX)

# check whether the current environment supports the C++23 standard
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
#if __cplusplus < 202302L
#error C++23 support is required
#endif

int main() {}"
  CXX23_SUPPORTED)

if (NOT CXX23_SUPPORTED)
  message(FATAL "C++23 support is required")
endif()

include(FetchContent)
FetchContent_Declare(
    execution26
    # for local development, use SOURCE_DIR <path-to>/execution26
    GIT_REPOSITORY https://github.com/bemanproject/execution26
    GIT_TAG 752882e
)
FetchContent_MakeAvailable(execution26)

# Add the beman::execution26 target if it is not already defined
if (NOT TARGET beman::execution26)
  find_package(beman_execution26 REQUIRED)
endif()

add_library(beman_task INTERFACE)
target_link_libraries(beman_task INTERFACE beman::execution26)

target_sources(beman_task
  INTERFACE
    FILE_SET beman_task_headers 
    TYPE HEADERS
    BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/src"
    FILES
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/task.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/any_env.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/any_scheduler.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/join_envs.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/manual_lifetime.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/query_base.hpp")

add_library(beman::task ALIAS beman_task)

if (PROJECT_IS_TOP_LEVEL)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
