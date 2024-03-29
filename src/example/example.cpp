#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../application.hpp"
#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "../obj_importer.hpp"

#include "timer.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "debug.hpp"
#include "model_import.hpp"

class MyGame : public application 
{
    public:

    MyGame(int win_w, int win_h) : application("MyGame", win_w, win_h, SDL_WINDOW_RESIZABLE) {}

    float dt = 0.0;

    camera player;
    glm::mat4 Projection;
    scene *apartment = nullptr;
    buffer UBO = buffer(GL_UNIFORM_BUFFER);
    buffer Camera_Buff = buffer(GL_UNIFORM_BUFFER);
    frame_timer timer;
    bool pause_rendering = false;
    bool escape = true;

    void init_gl_parameters()
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, nullptr);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void init()
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        init_gl_parameters();

        apartment = scene::import_scene("assets/apartment.gltf");
        if (!apartment){
            win.set_shouldClose(true);
            return;
        }
        apartment->loadShader("shader/deferred.vert.spv", "shader/deferred.frag.spv");
        apartment->setUBO();
        apartment->initMeshes();
        apartment->initDRenderer(win.getWidth(), win.getHeight());
        
        player.pos.y = 1.4f;
        player.makeView();
        Projection = glm::perspective(glm::radians(70.0f), win.getWidth() / (float)win.getHeight(), 0.1f, 1000.0f);
        UBO.generate();
        UBO.bufferData(3*sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        UBO.bindBufferBase(0);
        UBO.bufferSubData(sizeof(glm::mat4), 0, &Projection[0][0]);
        UBO.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.view[0][0]);
        Camera_Buff.generate();
        Camera_Buff.bufferData(32UL, nullptr, GL_DYNAMIC_DRAW);
        Camera_Buff.bindBufferBase(1);
    }

    void processKeyEvents(const SDL_KeyboardEvent &E)
    {
        bool pressed = E.type == SDL_KEYDOWN;
        switch(E.keysym.sym)
        {
            case SDLK_w:
            case SDLK_UP:
                player.update = pressed;
                player.movement.forward = pressed;
                break;

            case SDLK_s:
            case SDLK_DOWN:
                player.update = pressed;
                player.movement.backward = pressed;
                break;

            case SDLK_d:
            case SDLK_RIGHT:
                player.update = pressed;
                player.movement.right = pressed;
                break;

            case SDLK_a:
            case SDLK_LEFT:
                player.update = pressed;
                player.movement.left = pressed;
                break;

            case SDLK_SPACE:
                player.update = pressed;
                player.movement.up = pressed;
                break;

            case SDLK_LCTRL:
                player.update = pressed;
                player.movement.down = pressed;
                break;

            case SDLK_ESCAPE:
                if (pressed)
                    escape = !escape;
                break;
        }
    }

    void processMouseEvents(const SDL_MouseMotionEvent& E)
    {
        glm::vec2 r(E.xrel, -E.yrel);
        
        if (r != glm::vec2(0))
        {
            player.rotation += r*0.1f*dt;
            player.rotate();
        }
    }

    void resize_window(int w, int h)
    {
        win.set_size(w, h);
        apartment->get_DRenderer().resize(w,h);
        glViewport(0,0,w,h);
        float aspect = w / (float)h;
        Projection = glm::perspective(glm::radians(70.0f), aspect, 0.1f, 1000.0f);
        UBO.bufferSubData(sizeof(glm::mat4), 0, &Projection[0][0]);
    }

    void processWindowEvents(const SDL_WindowEvent &E)
    {
        switch(E.event)
        {
            case SDL_WINDOWEVENT_RESIZED:
                resize_window(E.data1, E.data2);
                break;
            case SDL_WINDOWEVENT_HIDDEN:
                pause_rendering = true;
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                pause_rendering = false;
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
                case SDL_MOUSEMOTION:
                    processMouseEvents(E.motion);
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    processKeyEvents(E.key);
                    break;
            }
        }
        SDL_SetRelativeMouseMode((SDL_bool)escape);
    }

    void mainLoop() override
    {
        while(!win.shouldClose())
        {
            timer.frameStart();

            process_events();

            player.move(dt);
            player.rotate();
            player.makeView();

            if (pause_rendering){
                dt = timer.frameEnd();
                continue;
            }

            Camera_Buff.bufferSubData(16, 0, &player.pos.x);
            Camera_Buff.bufferSubData(16, 16, &player.front.x);
            UBO.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.view[0][0]);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            apartment->render(UBO, 2*sizeof(glm::mat4));

            win.swapBuffer();
            dt = timer.frameEnd();
        }
    }

};

int main()
{
    MyGame App(500, 500);
    App.init();
    App.mainLoop();
    return 0;
}