#pragma once

#include "window.hpp"

class Application 
{
    public:

    Window win;

    Application(const std::string_view& title, int window_w, int window_h);
    Application(const std::string_view& title, int window_w, int window_h, int window_posx, int window_posy);
    Application(const std::string_view& title, int window_w, int window_h, int flags);
    Application(const std::string_view& title, int window_w, int window_h, int window_posx, int window_posy, int flags);
    
    virtual void processEvents() = 0;

    virtual void mainLoop() = 0;

    Application(Application&&) = delete;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    protected:
    void m_init();

};