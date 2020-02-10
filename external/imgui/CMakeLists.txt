cmake_minimum_required(VERSION 3.0)

project(imgui)

set(CMAKE_DEBUG_POSTFIX "d")

find_package(glew CONFIG)
find_package(glfw3 CONFIG)

set(SRC_FILES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_impl_glfw_gl3.cpp
)

set(HEADER_FILES
    imconfig.h
    imgui.h
    imgui_internal.h
    stb_rect_pack.h
    stb_textedit.h
    stb_truetype.h
    imgui_impl_glfw_gl3.h
)

add_library(imgui STATIC ${SRC_FILES} ${HEADER_FILES})

target_include_directories(imgui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GLEW_INCLUDE_DIR})
target_link_libraries(imgui PRIVATE glew_s glfw)

install(TARGETS imgui
#    EXPORT imguiTargets
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include/imgui
)

install(FILES ${HEADER_FILES} DESTINATION include/imgui)

#install(EXPORT imguiTargets
#    FILE imguiTargets.cmake
#    DESTINATION "lib/cmake/imgui"
#    NAMESPACE IMGUI::
#)

install(FILES imguiConfig.cmake
    DESTINATION "lib/cmake/imgui"
)