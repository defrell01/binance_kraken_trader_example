cmake_minimum_required(VERSION 3.10)
project(logger)

set(CMAKE_CXX_STANDARD 20)

set(HEADER_DIR ./)
set(SRC_DIR ./src)

file(GLOB HEADER_FILES ${HEADER_DIR}/*.hpp)
file(GLOB SOURCE_FILES ${SRC_DIR}/*.cpp)

add_library(logger STATIC ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(logger PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(logger PUBLIC ${CONAN_LIBS})