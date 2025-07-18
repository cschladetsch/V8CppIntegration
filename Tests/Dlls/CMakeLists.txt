cmake_minimum_required(VERSION 3.10)
project(V8Tests)

include(${CMAKE_SOURCE_DIR}/../../Source/v8Config.cmake)

add_executable(DirectIntegrationExample DirectIntegrationExample.cpp)
configure_v8_target(DirectIntegrationExample)

add_executable(ModuleExample ModuleExample.cpp)
configure_v8_target(ModuleExample)

add_executable(FibonacciTests FibonacciTests.cpp)
configure_v8_target(FibonacciTests)