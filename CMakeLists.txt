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
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/env_implementation.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/env_interface.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/env_wrapper.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/join_envs.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/manual_lifetime.hpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/beman/task/detail/query_base.hpp")

add_library(beman::task ALIAS beman_task)

if (PROJECT_IS_TOP_LEVEL)
  add_subdirectory(tests)
endif()