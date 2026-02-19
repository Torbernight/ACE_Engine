#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

#include "../application.hpp"
#include "../vertexArray.hpp"
#include "../buffer.hpp"

#include "debug/debug.hpp"
#include "misc/imgui_include.hpp"
#include "misc/timer.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "scene.hpp"
#include "physics.hpp"

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

class MyGame : public Application 
{
    public:
    MyGame(int win_w, int win_h) : Application("MyGame", win_w, win_h, SDL_WINDOW_RESIZABLE) {}

    uint64_t frame = 0;
    uint64_t ticks = 0;
    float runtime = 0.0;
    float dt = 0.0;
    float fps = 0.0;

    Mix_Chunk *footsteps_wave = nullptr;
    ImGuiIO* imgui_IO = nullptr;

    Camera player;
    Scene *gameScene = nullptr;

    Buffer TransformBuffer = Buffer(GL_UNIFORM_BUFFER);
    Buffer Camera_Buffer = Buffer(GL_UNIFORM_BUFFER);
    Buffer State_Buffer = Buffer(GL_UNIFORM_BUFFER);

    Timer frame_timer;

    bool pause_rendering = false;
    bool escape = false;
    bool vsync = false;
    bool showInfo = false;
    float mouse_sensitivity = 10.0f;
    float tick_rate = 1.0/60.0;
    enum MSAALevel{
        MS_OFF,
        MS_2,
        MS_4,
        MS_8,
        MS_NUM_OPTIONS
    } MSAA = MS_OFF;


    // initialization functions
    void initImGui();
    void initGLParameters();
    void initSDLParameters();
    void initAudio();
    void init();

    // events
    void processKeyEvents(const SDL_KeyboardEvent &E);
    void processMouseEvents(const SDL_MouseMotionEvent& E);
    void processWindowEvents(const SDL_WindowEvent &E);
    void processMouseWheel(const SDL_MouseWheelEvent& E);
    void processEvents() override;
    
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

    bool meshIntersects(const Mesh& M);
    DynamicObject* selectObject();
    void pointObject();
    void useObject();
    void checkSelectableObject();

    void updateGameLogic();

    inline void startFrame();
    inline void endFrame();
    inline void end_norender();

    void mainLoop() override;

    void cleanup();
};