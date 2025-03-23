cmake_minimum_required(VERSION 3.10)
project(orderbook)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES_DIR ./src)
set(HEADER_DIR ./)

file(GLOB SRC_FILES ${SOURCES_DIR}/*.cpp)
file(GLOB HEADER_FILES ${HEADER_DIR}/*.hpp)

add_library(orderbook STATIC ${SRC_FILES} ${HEADER_FILES})

set_target_properties(orderbook PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(orderbook PUBLIC ${CONAN_LIBS})
