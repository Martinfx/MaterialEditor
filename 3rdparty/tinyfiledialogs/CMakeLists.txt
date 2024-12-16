cmake_minimum_required(VERSION 3.7)

project(tinyfiledialogs)

add_library(${PROJECT_NAME}
      src/tinyfiledialogs.c
      include/tinyfiledialogs/tinyfiledialogs.h
)

add_library(::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/gen>
    PUBLIC $<INSTALL_INTERFACE:include>
)

add_subdirectory(test)

configure_file(${PROJECT_NAME}-config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake DESTINATION lib/cmake/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets DESTINATION lib)
install(DIRECTORY include DESTINATION .)
install(EXPORT ${PROJECT_NAME}-targets NAMESPACE :: DESTINATION lib/cmake/${PROJECT_NAME})
