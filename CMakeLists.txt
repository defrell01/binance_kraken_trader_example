cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 20) 

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(web_client/)
include_directories(./)

add_subdirectory(web_client/webSocketClient)

add_executable(main web_client/main.cpp)

target_link_libraries(main client ${CONAN_LIBS})
