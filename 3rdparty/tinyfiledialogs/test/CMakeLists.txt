cmake_minimum_required(VERSION 3.7)

project(tinyfiledialogs_examples)

add_subdirectory(hello)

if(WIN32)
    add_subdirectory(hello_wchar_t)
endif()
