cmake_minimum_required(VERSION 3.16)

project(materialeditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
include_directories(${IMGUI_INCLUDE_DIRS})
link_libraries(${IMGUI_LIBRARIES})
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(nlohmann_json REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
add_subdirectory("3rdparty/imnodes")
add_subdirectory("3rdparty/tinyfiledialogs")

add_executable(materialeditor main.cpp
    node.hpp
    link.hpp
    editor.hpp
    graph.hpp
    object.hpp
    shader.hpp
    framebuffer.hpp
    texture.hpp)

target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARIES} ${IMGUI_LIBRARIES} imnodes glfw imgui::imgui OpenGL::GL nlohmann_json::nlohmann_json tinyfiledialogs)

include(GNUInstallDirs)
install(TARGETS materialeditor
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
