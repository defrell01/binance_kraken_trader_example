cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 20) 

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(bot/)
include_directories(./)

add_subdirectory(bot/binanceWebSocketClient)
add_subdirectory(bot/krakenWebSocketClient)
add_subdirectory(bot/order_book)


add_executable(bot bot/main.cpp)

target_link_libraries(bot binaneClient krakenClient orderbook ${CONAN_LIBS})
