#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

#include "SDL_events.h"
#include "window.hpp"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "../renderer/vertexArray.hpp"
#include "../renderer/buffer.hpp"
#include "timer.hpp"
#include "../player.hpp"
#include "../scene/scene.hpp"

struct StateUBO
{
    uint64_t frame;
    float runtime;
    float delta;
    glm::vec2 resolution;
};
struct CameraUBO
{
    glm::vec4 pos;
    glm::vec4 front;
    float zNear;
    float zFar;
};

class Engine
{
    public:

    Window win;

    uint64_t frame  = 0ul;
    uint64_t ticks  = 0ul;
    float runtime   = 0.0f;
    float timeDelta = 0.0f;

    Mix_Chunk *footsteps_wave = nullptr; // TODO implementar sistema de audio aparte que maneje esto
    ImGuiIO* imgui_IO         = nullptr;

    Player player;
    Scene *currentScene = nullptr;

    Renderer::Buffer TransformBuffer = Renderer::Buffer(GL_UNIFORM_BUFFER);
    Renderer::Buffer Camera_Buffer   = Renderer::Buffer(GL_UNIFORM_BUFFER);
    Renderer::Buffer State_Buffer    = Renderer::Buffer(GL_UNIFORM_BUFFER);

    Timer frame_timer;

    bool pause_rendering = false;
    bool open_menu       = false;
    float tick_rate         = 1.0/60.0;

    Engine(const std::string& title, int window_w, int window_h);
    Engine(const std::string& title, int window_w, int window_h, int window_posx, int window_posy);
    Engine(const std::string& title, int window_w, int window_h, int flags);
    Engine(const std::string& title, int window_w, int window_h, int window_posx, int window_posy, int flags);
    
    Engine(Engine&&)                 = delete;
    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&)      = delete;


    void processEvents();
    void mainLoop();

    // initialization functions
    void initImGui();
    void initGLParameters();
    void initSDLParameters();
    void initAudio();
    void init();

    void processWindowEvents(const SDL_WindowEvent &event);
    
    void resizeWindow(int w, int h);
    void setVSync();

    void showConfig();
    void infoWindow();
    static void imguiFrameStart();
    static void imguiFrameEnd();

    void updateUniformBuffers();

    void testCollision();
    void updatePhysics();

    void frustumTest();

    bool meshIntersects(const Renderer::Mesh& M);
    DynamicObject* selectObject();
    void pointObject();
    void useObject();
    void checkSelectableObject();

    void updateGameLogic();

    inline void startFrame();
    inline void endFrame();
    inline void endFrameWithoutRendering();


    void cleanup();
};