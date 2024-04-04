#include <iostream>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../application.hpp"
#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "../obj_importer.hpp"

#include "imgui_include.hpp"
#include "timer.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "debug.hpp"
#include "scene.hpp"

class MyGame : public application 
{
    public:

    MyGame(int win_w, int win_h) : application("MyGame", win_w, win_h, SDL_WINDOW_RESIZABLE) {}

    float dt = 0.0;
    uint64_t frame = 0;
    camera player;
    scene *gameScene = nullptr;
    buffer TransformBuffer = buffer(GL_UNIFORM_BUFFER);
    buffer Camera_Buff = buffer(GL_UNIFORM_BUFFER);
    Timer frame_timer, app_timer;
    bool pause_rendering = false;
    bool escape = false;
    bool vsync = false;

    ImGuiIO* imgui_IO = nullptr;

    void init_imgui()
    {
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForOpenGL(this->win.getHandle(), this->win.getContext());
        ImGui_ImplOpenGL3_Init();
        imgui_IO = &ImGui::GetIO();
    }

    void init_gl_parameters()
    {
#ifndef NDEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, nullptr);
#endif
        glEnable(GL_PRIMITIVE_RESTART);
        glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void init()
    {
        app_timer.Start();
        SDL_SetRelativeMouseMode(SDL_TRUE);
        init_gl_parameters();
        init_imgui();

        gameScene = scene::import_scene("assets/test.gltf");
        if (!gameScene){
            win.set_shouldClose(true);
            return;
        }
        gameScene->loadShader("shader/deferred.vert.spv", "shader/deferred.frag.spv");
        gameScene->setUBO();
        gameScene->initMeshes();
        gameScene->initDRenderer(win.getWidth(), win.getHeight());

        if (!glm::any(glm::isnan(gameScene->get_defaultCamPos())))
        {
            player.setPos(gameScene->get_defaultCamPos());
        }
        
        player.makeView();
        player.makeProj(win.getWidth() / (float)win.getHeight());
        TransformBuffer.generate();
        TransformBuffer.bufferData(3*sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        TransformBuffer.bindBufferBase(0);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView()[0][0]);
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
                player.movement.forward = pressed;
                break;

            case SDLK_s:
            case SDLK_DOWN:
                player.movement.backward = pressed;
                break;

            case SDLK_d:
            case SDLK_RIGHT:
                player.movement.right = pressed;
                break;

            case SDLK_a:
            case SDLK_LEFT:
                player.movement.left = pressed;
                break;

            case SDLK_SPACE:
                player.movement.up = pressed;
                break;

            case SDLK_LCTRL:
                player.movement.down = pressed;
                break;
            
            case SDLK_LSHIFT:
                player.movement.sprint = pressed;
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
            player.rotate(r*0.1f, dt);
    }

    void resize_window(int w, int h)
    {
        win.set_size(w, h);
        gameScene->resizeViewport(w,h);
        glViewport(0,0,w,h);
        float aspect = w / (float)h;
        player.makeProj(aspect);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
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

    void setVSync()
    {
        if(SDL_GL_SetSwapInterval(vsync ? -1 : 0))
            SDL_GL_SetSwapInterval(vsync ? 1 : 0);
    }

    void show_config()
    {
        ImGui::Begin("Config", &escape);

        ImGui::Text("Frame: %lu", frame);
        ImGui::Text("Runtime: %f", app_timer.Count());
        ImGui::Text("deltaTime: %f", dt);
        ImGui::Text("fps: %.2f", 1.0/dt);

        if (ImGui::Checkbox("VSync", &vsync))
            setVSync();

        ImGui::SeparatorText("Post Processing");

        uint8_t postProc_Update = 0;
        postProc_Update |= ImGui::DragFloat("Brightness", &gameScene->postprocess.Brightness, 0.025, -1.0, 2.0);
        postProc_Update |= ImGui::DragFloat("Contrast", &gameScene->postprocess.Contrast, 0.025, 0.0, 2.0);
        postProc_Update |= ImGui::DragFloat("Saturation", &gameScene->postprocess.Saturation, 0.025, 0.0, 4.0);
        postProc_Update |= ImGui::DragFloat("Gamma", &gameScene->postprocess.Gamma, 0.025, 0.0, 4.0);
        postProc_Update |= ImGui::DragFloat("Exposure", &gameScene->postprocess.Exposure, 0.1, FLT_MIN, FLT_MAX);

        if (postProc_Update)
            gameScene->updatePostProcessBuffer();

        ImGui::End();
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
                    if(imgui_IO->WantCaptureMouse){
                        ImGui_ImplSDL2_ProcessEvent(&E);
                        break;
                    }
                    processMouseEvents(E.motion);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                if (imgui_IO->WantCaptureMouse){
                    ImGui_ImplSDL2_ProcessEvent(&E);
                    break;
                }
                break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    if (imgui_IO->WantCaptureKeyboard){
                        ImGui_ImplSDL2_ProcessEvent(&E);
                        break;
                    }
                    processKeyEvents(E.key);
                    break;
                case SDL_TEXTINPUT:
                    if (imgui_IO->WantTextInput){
                        ImGui_ImplSDL2_ProcessEvent(&E);
                    }
                    break;
            }
        }
        SDL_SetRelativeMouseMode((SDL_bool)(!escape));
    }

    void imgui_frame_start()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void imgui_frame_end()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void update_UBOs()
    {
        glm::vec3 playerdata[2] = {player.getPos(), player.getFront()}; 
        Camera_Buff.bufferSubData(32, 0, playerdata);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView());
    }

    void mainLoop() override
    {
        while(!win.shouldClose())
        {
            frame_timer.Start();

            imgui_frame_start();

            process_events();

            if(escape) {
                show_config();

            } else {
                player.move(dt);
                player.makeView();
            }

            if (pause_rendering) {
                dt = frame_timer.Count();
                continue;
            }

            update_UBOs();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            gameScene->render(TransformBuffer, 2*sizeof(glm::mat4));

            imgui_frame_end();

            win.swapBuffer();
            frame++;
            dt = frame_timer.Count();
        }
    }

    void cleanup()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

};

int main()
{
    MyGame App(500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}