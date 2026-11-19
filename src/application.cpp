#include <iostream>
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "input/input.hpp"
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

void Application::processEvents()
{
    InputHandler &input = InputHandler::get();
    SDL_Event E;
    while (SDL_PollEvent(&E))
    {
        switch(E.type) 
        {
            case SDL_QUIT:
                win.set_shouldClose(true);
                break;
            case SDL_WINDOWEVENT:
                processWindowEvents(E.window);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
            case SDL_MOUSEMOTION:
                input.process_mouse_event(E);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                input.process_keyboard_event(E.key);
                break;
            case SDL_TEXTINPUT:
                break;
        }
    }
}
