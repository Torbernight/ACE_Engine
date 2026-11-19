#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <execution>

#include "editor_camera.hpp"
#include "../application.hpp"
#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "../misc/imgui_include.hpp"
#include "../misc/timer.hpp"
#include "../camera.hpp"
#include "../material.hpp"
#include "../debug.hpp"
#include "../scene.hpp"
#include "../extensions.hpp"
//#include "../physics.hpp"

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

class Game : public application 
{
    public:

    Game(int win_w, int win_h) : application("MyGame", win_w, win_h, SDL_WINDOW_RESIZABLE) {}

    uint64_t frame = 0;
    uint64_t ticks = 0;
    float runtime = 0.0;
    float dt = 0.0;
    float fps = 0.0;

    Mix_Chunk *footsteps_wave = nullptr;

    editor_camera player;
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

    ImGuiIO* imgui_IO = nullptr;

    void init_imgui()
    {
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForOpenGL(this->win.getHandle(), this->win.getContext());
        ImGui_ImplOpenGL3_Init("#version 460");
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
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    void init()
    {
        if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) < 0) {
            SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
            win.set_shouldClose(true);
            return;
        }
        footsteps_wave = Mix_LoadWAV("assets/sound/silly_footstep.wav");
        if (!footsteps_wave) {
            SDL_Log("Couldn't load 'assets/sound/silly_footstep.wav': %s\n", SDL_GetError());
            win.set_shouldClose(true);
            return;
        }
        
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_GL_SetSwapInterval(0);
        init_gl_parameters();
        init_imgui();

        gameScene = scene::import_scene("assets/garden.gltf");
        if (!gameScene){
            win.set_shouldClose(true);
            return;
        }
        gameScene->load_SceneShader("shader/scene.vert.spv", "shader/scene.frag.spv");
        gameScene->load_DeferredShader("shader/deferred.vert.spv", "shader/editor_deferred.frag.spv");
        gameScene->load_PostProcShader("shader/deferred.vert.spv", "shader/postprocess.frag.spv");
        gameScene->initSkybox("shader/skybox.vert.spv", "shader/skybox.frag.spv", "ps.tif");
        gameScene->setUBO();
        gameScene->initMeshes();
        gameScene->initDRenderer(win.getWidth(), win.getHeight());

        if (!glm::any(glm::isnan(gameScene->get_defaultCamPos())))
        {
            player.setPos(gameScene->get_defaultCamPos());
        }
        
        player.makeView();
        player.setAspect(win.getWidth() / (float)win.getHeight());
        player.makeProj();
        TransformBuffer.generate();
        TransformBuffer.bufferData((2+gameScene->getMaxInstances())*sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        TransformBuffer.bindBufferBase(0);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView()[0][0]);
        Camera_Buffer.generate();
        Camera_Buffer.bufferData(sizeof(camera_UBO), nullptr, GL_DYNAMIC_DRAW);
        Camera_Buffer.bindBufferBase(1);
        state_UBO buff = {};
        State_Buffer.generate();
        State_Buffer.bufferData(sizeof(state_UBO), &buff, GL_DYNAMIC_DRAW);
        State_Buffer.bindBufferBase(5);

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

            case SDLK_e:
                player.movement.use = !player.movement.use_key && pressed;
                player.movement.use_key = pressed;
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
        player.m.delta_rotation = r*mouse_sensitivity*0.01f;
    }

    void resize_window(int w, int h)
    {
        win.set_size(w, h);
        gameScene->resizeViewport(w,h);
        glViewport(0,0,w,h);
        float aspect = w / (float)h;
        player.setAspect(aspect);
        player.makeProj();
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

        ImGui::Checkbox("Show Info", &showInfo);

        if (ImGui::Checkbox("VSync", &vsync))
            setVSync();

        ImGui::SeparatorText("Controls");

        ImGui::DragFloat("Mouse Sensitivity", &mouse_sensitivity, 1.0f, 1.0f, 100.0f);

        ImGui::SeparatorText("Post Processing");

        uint8_t postProc_Update = 0;
        postProc_Update |= ImGui::DragFloat("Brightness", &gameScene->postprocess.Brightness, 0.025, -1.0, 2.0);
        postProc_Update |= ImGui::DragFloat("Contrast", &gameScene->postprocess.Contrast, 0.025, 0.0, 2.0);
        postProc_Update |= ImGui::DragFloat("Saturation", &gameScene->postprocess.Saturation, 0.025, 0.0, 4.0);
        postProc_Update |= ImGui::DragFloat("Gamma", &gameScene->postprocess.Gamma, 0.025, 0.0, 4.0);
        postProc_Update |= ImGui::DragFloat("Exposure", &gameScene->postprocess.Exposure, 0.1, -5.0, 5.0);

        uint8_t camera_update = 0;
        ImGui::SeparatorText("Camera options");
        camera_update |= ImGui::DragFloat("near plane", &player.m.zNear);
        camera_update |= ImGui::DragFloat("far plane", &player.m.zFar);
        camera_update |= ImGui::DragFloat("FOV", &player.m.FOVy, 0.01f, M_PIf/16.0f, M_PIf/2.0f);

        ImGui::SeparatorText("Scene Info");

        for (const auto &M : gameScene->Meshes) {
            ImGui::Text("Mesh name: %s", M.m.Name.c_str());
            ImGui::Text("visible: %u", M.m.visible);
            ImGui::Separator();
        }

        if (postProc_Update)
            gameScene->updatePostProcessBuffer();
        if (camera_update){
            player.makeProj();
            TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
            
        }

        ImGui::End();
    }

    void process_Mousewheel(const SDL_MouseWheelEvent& E)
    {
        float FOV = glm::radians(glm::degrees(player.getFOV())+E.y);
        player.setFOV(glm::clamp(FOV,0.0f, M_PIf));
        TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
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
                if (E.button.button == SDL_BUTTON_LEFT) {
                    uint8_t click = E.type == SDL_MOUSEBUTTONDOWN;
                    player.movement.hasClicked = player.movement.lclick != click && click;
                    player.movement.lclick = click;
                }
                break;
                case SDL_MOUSEWHEEL:
                if (imgui_IO->WantCaptureMouse){
                    ImGui_ImplSDL2_ProcessEvent(&E);
                    break;
                }
                //process_Mousewheel(E.wheel);
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
        camera_UBO camBuff = {glm::vec4(player.getPos(), 0), glm::vec4(player.getFront(), 0), player.getNear(), player.getFar()};
        Camera_Buffer.bufferSubData(sizeof(camBuff), 0, &camBuff);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView());
        state_UBO stateBuff = {frame, runtime, dt, {win.getWidth()*0.5f, win.getHeight()*0.5f}};
        State_Buffer.bufferSubData(sizeof(stateBuff), 0, &stateBuff);
    }
#define XYZ(V) V.x, V.y, V.z
#define XY(V) V.x, V.y
    void info_window()
    {
        if (!showInfo)
            return;
        ImGui::SetNextWindowBgAlpha(0.5);
        ImGui::Begin("Info", &showInfo);
        ImGui::Text("Frame: %lu", frame);   
        ImGui::Text("Runtime: %fs", runtime);
        ImGui::Text("%f ms", (1.0/fps)*1e+3);
        ImGui::Text("%.2f fps", fps);
        ImGui::Text("position: %f %f %f", XYZ(player.m.pos));
        ImGui::Text("angle: %f %f", XY(glm::degrees(player.m.rotation)));
        ImGui::End();
    }
#undef XYZ
#undef XY

    bool meshIntersects(const mesh& M)
    {
        glm::vec2 intersection = (M.m.bounding_box*M.m.transform).ray_intersection(player.getPos(), player.getFront());
        return intersection.x <= intersection.y && intersection.x <= 3.0f;
    }

    dynamic_object* select_object()
    {
        float dist = FLT_MAX;
        auto M = std::find_if(gameScene->dynamic_objects.begin(), gameScene->dynamic_objects.end(), 
        [this, &dist](const dynamic_object& Obj)
        {
            if (Obj.m_grabbed)
                return false;
            auto &M = gameScene->Meshes[Obj.m.Mesh];
            float d = glm::distance(player.m.pos, (M.m.bounding_box*M.m.transform).get_center());
            bool closest = d < dist;
            //I don't think this works the way I intend it to
            dist = closest ? d : dist;
            return meshIntersects(M) && closest;
        });
        return M == gameScene->dynamic_objects.end() ? nullptr : &(*M);
    }

    void point_object()
    {
        dynamic_object* object = player.m.pointed_object;

        if (object)
            return;

        if ((object = select_object()))
            object->m_selected = true;

        player.m.pointed_object = object;
    }

    void use_object()
    {
        dynamic_object* grabbed = player.m.grabbed_object; 
        if (!grabbed){
            if ((grabbed = player.m.pointed_object)){
                grabbed->grab(&player);
            }
            return;
        }
        grabbed->use(&player);
    }

    void check_selectable_object()
    {
        point_object();
        if (player.movement.use)
            use_object();
    }

    void check_on_frustum()
    {
        std::for_each(std::execution::par, gameScene->Meshes.begin(), gameScene->Meshes.end(), [this](mesh& M){M.onFrustum(player.m.view_frustum);});
    }

    void update_gameLogic()
    {
        static float accumulated = 0.0f;
        accumulated += dt;

        while (accumulated >= tick_rate)
        {
            player.reset_rotation();
            process_events();
            
            if (escape)
                goto skipLoop;
            
            check_selectable_object();
            player.move(tick_rate);
            player.rotate(tick_rate);

skipLoop:
            accumulated -= tick_rate;
            ticks++;
            if (!(ticks%20))
                fps = 1.0/dt;
        }

        player.makeView();
        check_on_frustum();
    }

    inline void startFrame()
    {
        frame_timer.Start();
        imgui_frame_start();
    }

    inline void endFrame()
    {
        imgui_frame_end();
        win.swapBuffer();
        frame++;
        runtime += (dt = frame_timer.Count());
    }

    inline void end_norender()
    {
        runtime += (dt = frame_timer.Count());
    }

    void mainLoop() override
    {
        while(!win.shouldClose())
        {
            startFrame();

            update_gameLogic();

            if(escape)
                show_config();

            info_window();
            
            if (pause_rendering) {
                end_norender();
                continue;
            }

            update_UBOs();

            gameScene->render(TransformBuffer, 2*sizeof(glm::mat4));

            endFrame();
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
    Game App(500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}