#pragma once

#include "window.hpp"

class application 
{
    public:

    window win;

    application(const std::string_view& title, int window_w, int window_h);
    application(const std::string_view& title, int window_w, int window_h, int window_posx, int window_posy);
    application(const std::string_view& title, int window_w, int window_h, int flags);
    application(const std::string_view& title, int window_w, int window_h, int window_posx, int window_posy, int flags);
    
    virtual void process_events() = 0;

    virtual void mainLoop() = 0;

    application(application&&) = delete;
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application& operator=(application&&) = delete;

    protected:
    void m_init();

};