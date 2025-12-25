cmake_minimum_required(VERSION 3.10...4.20)

project(PollServer)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(PollServer
  "${CMAKE_SOURCE_DIR}/src/pollserver.c"
  "${CMAKE_SOURCE_DIR}/src/main.c"
)

target_include_directories(PollServer PRIVATE "${CMAKE_SOURCE_DIR}/src/include")
