
#include <SDL2/SDL.h>
#include "window.hpp"


Window::Window(const std::string_view& title, int width, int height, int posx, int posy)
: m(title, width, height, posx, posy, 0)
{
    m_init();
}
Window::Window(const Window::members& mstruct)
: m(mstruct)
{
    m_init();
}

Window::Window(const std::string_view& title, int width, int height, int posx, int posy, int flags)
: m(title, width, height, posx, posy, flags)
{
    m_init();
}

Window::~Window()
{
    SDL_GL_DeleteContext(m.GL_Context);
    SDL_DestroyWindow(m.SDL_Win);
    SDL_Quit();
}

int Window::m_init()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    m.SDL_Win = SDL_CreateWindow(m.title.c_str(), m.posx, m.posy, m.width, m.height, SDL_WINDOW_OPENGL | m.flags);
    m.GL_Context = SDL_GL_CreateContext(m.SDL_Win);
    return 0;
}


void Window::set_title(const std::string_view& t)
{
    m.title = t;
    SDL_SetWindowTitle(m.SDL_Win, m.title.c_str());
}
void Window::set_size(int w, int h)
{
    m.width = w;
    m.height = h;
}
void Window::set_width(int w)
{
    m.width = w;
}
void Window::set_height(int h)
{
    m.height = h;
}
void Window::swapBuffer()
{
    SDL_GL_SwapWindow(m.SDL_Win);
}
void Window::setSwapInterval(int s)
{
    SDL_GL_SetSwapInterval(s);
}