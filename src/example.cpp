#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "application.hpp"

class MyGame : public application 
{
    public:

    MyGame(int win_w, int win_h) : application("MyGame", win_w, win_h, SDL_WINDOW_RESIZABLE) {}

    void init()
    {

    }

    void processWindowEvents(const SDL_WindowEvent &E)
    {
        switch(E.type)
        {
            case SDL_WINDOWEVENT_RESIZED:
                win.set_size(E.data1, E.data2);
                glViewport(0,0,E.data1,E.data2);
                break;
        }
    }

    void process_events() override
    {
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
            }
        }
    }

    void mainLoop() override
    {
        init();
        while(!win.shouldClose())
        {
            process_events();

            glClear(GL_COLOR_BUFFER_BIT);

            win.swapBuffer();
        }
    }

};

int main()
{
    MyGame App(500, 500);
    
    App.mainLoop();

    return 0;
}