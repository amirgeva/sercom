cmake_minimum_required(VERSION 3.16)
project(sercom)

file(GLOB SRCS include/* src/*)

if (UNIX)
add_definitions("-DUNIX")
endif (UNIX)

add_library(sercom ${SRCS})
target_include_directories(sercom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
#target_link_libraries(ht PRIVATE glfw glad::glad)

add_subdirectory(sample)
