cmake_minimum_required(VERSION 4.5.1)

add_library(glad STATIC)

target_sources(
    glad

    PRIVATE
        gl.c

    PUBLIC
        FILE_SET glad_headers
        TYPE HEADERS
        BASE_DIRS 
            include/
        FILES
            include/glad/gl.h
            include/KHR/khrplatform.h
    
)

add_library(Imgui STATIC)

target_sources(
    Imgui

    PRIVATE
        include/imgui/imgui.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/imgui_tables.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_demo.cpp

    PUBLIC 
        FILE_SET imgui_headers
        TYPE HEADERS
        BASE_DIRS
            include/imgui
        FILES
            include/imgui/imconfig.h
            include/imgui/imgui_internal.h
            include/imgui/imgui.h
            include/imgui/imstb_rectpack.h
            include/imgui/imstb_textedit.h
            include/imgui/imstb_truetype.h
)

add_library(Imgui_backend STATIC)

target_include_directories(Imgui_backend PUBLIC include/imgui include/ /usr/include/SDL2)


target_sources(
    Imgui_backend

    PRIVATE
        include/imgui/backends/imgui_impl_opengl3.cpp
        include/imgui/backends/imgui_impl_sdl2.cpp

    PUBLIC 
        FILE_SET imgui_backend_headers
        TYPE HEADERS
        BASE_DIRS
            include/imgui/backends
        FILES
            include/imgui/backends/imgui_impl_opengl3.h
            include/imgui/backends/imgui_impl_sdl2.h
)

