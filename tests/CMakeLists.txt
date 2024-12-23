# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
cmake_minimum_required(VERSION 3.28...3.31)

project(beman_task_tests VERSION 1.0 LANGUAGES CXX)

if (NOT TARGET beman::task)
  find_package(beman_task)
endif()

add_executable(task.test task.test.cpp)
target_link_libraries(task.test PRIVATE beman::task)

add_test(NAME task COMMAND task.test)