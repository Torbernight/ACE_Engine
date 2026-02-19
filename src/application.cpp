#include <iostream>
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "application.hpp"

void Application::m_init()
{
    glewExperimental = GL_TRUE;
    int status = glewInit();
    if (status != GLEW_OK){
        std::cerr << "Fatal error: couldn't initialize GLEW library:\n\t" << glewGetErrorString(status) << std::endl;
    }
    glClearColor(0.3, 0.5, 0.4, 1.0);
}

Application::Application(const std::string_view& title, int window_w, int window_h)
: win(title, window_w, window_h, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED)
{
    m_init();
}
Application::Application(const std::string_view& title, int window_w, int window_h, int flags)
: win(title, window_w, window_h, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, flags)
{
    m_init();
}
Application::Application(const std::string_view& title, int window_w, int window_h, int window_posx, int window_posy, int flags)
: win(title, window_w, window_h, window_posx, window_posy, flags)
{
    m_init();
}

Application::Application(const std::string_view& title, int window_w, int window_h, int window_posx, int window_posy)
: win(title, window_w, window_h, window_posx, window_posy)
{
    m_init();
}
