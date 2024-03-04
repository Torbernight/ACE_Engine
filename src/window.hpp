#pragma once

#include <string>

typedef struct SDL_Window SDL_Window;

class window {
    private:
    struct members;
    public:


    window(const std::string_view& title, int width, int height, int posx, int posy);
    window(const std::string_view& title, int width, int height, int posx, int posy, int flags);
    window(const members& mstruct);
    ~window();

    window& operator=(const window&) = delete;
    window& operator=(window&&) = delete;
    window(const window&) = delete;
    window(window&&) = delete;

    SDL_Window* getHandle() const {return m.SDL_Win;}
    void* getContext() const {return m.GL_Context;}
    int getWidth() const {return m.width;}
    int getHeight() const {return m.height;}
    int getPosX() const {return m.posx;}
    int getPosY() const {return m.posy;}
    const char* get_title() const {return m.title.c_str();}

    bool shouldClose() const {return m.should_close;}

    void set_title(const std::string_view& t);
    void set_size(int w, int h);
    void set_width(int w);
    void set_height(int w);
    void set_shouldClose(bool shouldClose) {m.should_close = shouldClose;}

    void swapBuffer();
    void setSwapInterval(int s);

    private:
    struct members 
    {
        int width, height;
        int posx, posy;
        std::string title;

        bool should_close = false;
        SDL_Window* SDL_Win = 0;
        void* GL_Context = 0;

        int flags;

        members(const std::string_view& t, int w, int h, int x, int y, int Flags) :
        width(w), height(h), posx(x), posy(y), title(t), flags(Flags)
        {}
    } m;

    int m_init();

};