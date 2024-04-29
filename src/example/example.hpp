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

struct state_UBO
{
    uint64_t frame;
    float runtime;
    float delta;
    glm::vec2 resolution;
};
struct camera_UBO
{
    glm::vec4 pos;
    glm::vec4 front;
    float zNear;
    float zFar;
};

class MyGame : public application 
{
    public:
    MyGame(int win_w, int win_h) : application("MyGame", win_w, win_h, SDL_WINDOW_RESIZABLE) {}

    uint64_t frame = 0;
    uint64_t ticks = 0;
    float runtime = 0.0;
    float dt = 0.0;
    float fps = 0.0;

    Mix_Chunk *footsteps_wave = nullptr;
    ImGuiIO* imgui_IO = nullptr;


    camera player;
    scene *gameScene = nullptr;

    buffer TransformBuffer = buffer(GL_UNIFORM_BUFFER);
    buffer Camera_Buffer = buffer(GL_UNIFORM_BUFFER);
    buffer State_Buffer = buffer(GL_UNIFORM_BUFFER);

    Timer frame_timer;

    bool pause_rendering = false;
    bool escape = false;
    bool vsync = false;
    bool showInfo = false;
    float mouse_sensitivity = 10.0f;
    float tick_rate = 1.0/60.0;


    // initialization functions
    void init_imgui();
    void init_gl_parameters();
    void init_sdl_parameters();
    void init_audio();
    void init();

    // events
    void processKeyEvents(const SDL_KeyboardEvent &E);
    void processMouseEvents(const SDL_MouseMotionEvent& E);
    void processWindowEvents(const SDL_WindowEvent &E);
    void process_Mousewheel(const SDL_MouseWheelEvent& E);
    void process_events() override;
    
    void resize_window(int w, int h);
    void setVSync();

    void show_config();
    void info_window();
    static void imgui_frame_start();
    static void imgui_frame_end();

    void update_UBOs();

    void test_collision();
    void update_physics();

    void check_on_frustum();

    bool meshIntersects(const mesh& M);
    dynamic_object* select_object();
    void point_object();
    void use_object();
    void check_selectable_object();

    void update_gameLogic();

    inline void startFrame();
    inline void endFrame();
    inline void end_norender();

    void mainLoop() override;

    void cleanup();
};