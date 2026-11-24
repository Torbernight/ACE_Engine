cmake_minimum_required(VERSION 4.5.1)


target_sources(
    ACE

    PRIVATE
        main.cpp
        player.cpp

    PRIVATE
        FILE_SET player_header
        TYPE HEADERS
        FILES
            player.hpp
)


target_sources(
    ACE

    PRIVATE
        core/engine.cpp
        core/settings.cpp
        core/window.cpp

    PUBLIC 
        FILE_SET core_headers
        TYPE HEADERS
        BASE_DIRS
            core/
        FILES
            core/engine.hpp
            core/settings.hpp
            core/timer.hpp
            core/window.hpp
)

target_sources(
    ACE

    PRIVATE
        input/input.cpp

    PUBLIC 
        FILE_SET input_headers
        TYPE HEADERS
        BASE_DIRS
            input/
        FILES
            input/input.hpp
)

target_sources(
    ACE

    PRIVATE
        physics/physics.cpp

    PUBLIC 
        FILE_SET physics_headers
        TYPE HEADERS
        BASE_DIRS
            physics/
        FILES
            physics/physics.hpp
)

target_sources(
    ACE

    PRIVATE
        renderer/buffer.cpp
        renderer/camera.cpp
        renderer/canvas_renderer.cpp
        renderer/cubemap_renderer.cpp
        renderer/framebuffer.cpp
        renderer/material.cpp
        renderer/mesh.cpp
        renderer/shader.cpp
        renderer/texture_load.cpp
        renderer/texture.cpp
        renderer/vertexArray.cpp


    PUBLIC 
        FILE_SET renderer_headers
        TYPE HEADERS
        BASE_DIRS
            renderer/
        FILES
            renderer/buffer.hpp
            renderer/camera.hpp
            renderer/canvas_renderer.hpp
            renderer/cubemap_renderer.hpp
            renderer/framebuffer.hpp
            renderer/material.hpp
            renderer/mesh.hpp
            renderer/shader.hpp
            renderer/shader_compiler.hpp
            renderer/texture_load.hpp
            renderer/texture.hpp
            renderer/vertexArray.hpp
            renderer/light.hpp
)

target_sources(
    ACE

    PRIVATE
        scene/scene.cpp
        scene/object.cpp

    PUBLIC 
        FILE_SET scene_headers
        TYPE HEADERS
        BASE_DIRS
            scene/
        FILES
            scene/scene.hpp
            scene/object.hpp
)

