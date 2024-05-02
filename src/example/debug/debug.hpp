#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <iostream>

inline static
bool IsExtensionSupported(const char *name)
{
    GLint n = 0; 
    glGetIntegerv(GL_NUM_EXTENSIONS, &n); 
    for (GLint i = 0; i < n; i++){ 
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if (!strcmp(name, extension)){
          return true;
        }
    }
    return false;
}

inline static
void BREAKPOINT()
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_assert(false);
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
    if (type != GL_DEBUG_TYPE_ERROR){
        return;
    }

  fprintf( stderr, "\nGL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );

    BREAKPOINT();
}
