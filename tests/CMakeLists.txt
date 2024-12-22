cmake_minimum_required(VERSION 3.28...3.31)

project(beman_task_tests VERSION 1.0 LANGUAGES CXX)

if (NOT TARGET beman::task)
  find_package(beman_task)
endif()

include(CTest)
enable_testing()

add_executable(task task.test.cpp)
target_link_libraries(task PRIVATE beman::task)