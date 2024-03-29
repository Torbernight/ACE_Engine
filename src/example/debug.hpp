#pragma once
#include <iostream>
#include <SDL2/SDL.h>

inline static
void BREAKPOINT()
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
    std::cout << "Breakpoint Triggered!\n";
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

inline static
void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );

    if (type == GL_DEBUG_TYPE_ERROR) 
        BREAKPOINT();
}
