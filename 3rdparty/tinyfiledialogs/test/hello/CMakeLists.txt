cmake_minimum_required(VERSION 3.7)

project(tinyfiledialogs_hello)

if(NOT TARGET ::tinyfiledialogs)
    find_package(tinyfiledialogs REQUIRED)
endif()

add_executable(${PROJECT_NAME} hello.c)
target_link_libraries(${PROJECT_NAME} PRIVATE ::tinyfiledialogs)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
