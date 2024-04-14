#include <iostream>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <execution>

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
#include "extensions.hpp"

struct state_UBO
{
    uint64_t frame;
    float runtime;
    float delta;
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
    float runtime = 0.0;
    float dt = 0.0;

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
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    void init()
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        init_gl_parameters();
        init_imgui();

        gameScene = scene::import_scene("assets/test.gltf");
        if (!gameScene){
            win.set_shouldClose(true);
            return;
        }
        gameScene->load_SceneShader("shader/scene.vert.spv", "shader/scene.frag.spv");
        gameScene->load_DeferredShader("shader/deferred.vert.spv", "shader/deferred.frag.spv");
        gameScene->load_PostProcShader("shader/deferred.vert.spv", "shader/postprocess.frag.spv");
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

        ImGui::SeparatorText("Post Processing");

        uint8_t postProc_Update = 0;
        postProc_Update |= ImGui::DragFloat("Brightness", &gameScene->postprocess.Brightness, 0.025, -1.0, 2.0);
        postProc_Update |= ImGui::DragFloat("Contrast", &gameScene->postprocess.Contrast, 0.025, 0.0, 2.0);
        postProc_Update |= ImGui::DragFloat("Saturation", &gameScene->postprocess.Saturation, 0.025, 0.0, 4.0);
        postProc_Update |= ImGui::DragFloat("Gamma", &gameScene->postprocess.Gamma, 0.025, 0.0, 4.0);
        postProc_Update |= ImGui::DragFloat("Exposure", &gameScene->postprocess.Exposure, 0.1, FLT_MIN, 1.0e+5);

        uint8_t camera_update = 0;
        ImGui::SeparatorText("Camera options");
        camera_update |= ImGui::DragFloat("near plane", &player.m.zNear);
        camera_update |= ImGui::DragFloat("far plane", &player.m.zFar);
        camera_update |= ImGui::DragFloat("FOV", &player.m.FOVy, 0.01f, M_PIf/16.0f, M_PIf/2.0f);

        ImGui::SeparatorText("Scene Info");

        for (const auto &M : gameScene->Meshes) {
            if (M.discard)
                continue;
            ImGui::Text("Mesh name: %s", M.Name.c_str());
            ImGui::Text("instances: %u", M.nInstances);
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
        state_UBO stateBuff = {frame, runtime, dt};

        TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView());
        Camera_Buffer.bufferSubData(sizeof(camBuff), 0, &camBuff);
        State_Buffer.bufferSubData(sizeof(stateBuff), 0, &stateBuff);
    }
#define XYZ(V) V.x, V.y, V.z
#define XY(V) V.x, V.y
    void info_window()
    {
        if (!showInfo)
            return;
        ImGui::Begin("Info", &showInfo);
        ImGui::Text("Frame: %lu", frame);   
        ImGui::Text("Runtime: %fs", runtime);
        ImGui::Text("%f ms", dt*1e+3);
        ImGui::Text("%.2f fps", 1.0/dt);
        ImGui::Text("position: %f %f %f", XYZ(player.m.pos));
        ImGui::Text("speed: %f %f %f", XYZ(player.m.realVel));
        ImGui::Text("angle: %f %f", XY(glm::degrees(player.m.rotation)));
        ImGui::Text("collision: %s",player.m.collision ? "true" : "false");
        ImGui::End();
    }
#undef XYZ
#undef XY

    inline static 
    float maxcomp(const glm::vec3& V){
        return glm::max(abs(V.x), glm::max(abs(V.y),abs(V.z)));
    }
    inline static
    void clampnan(glm::vec3 &V){
        V.x = isnan(V.x) ? 0.0 : isinf(V.x) ? glm::sign(V.x) : V.x;
        V.y = isnan(V.y) ? 0.0 : isinf(V.y) ? glm::sign(V.y) : V.y;
        V.z = isnan(V.z) ? 0.0 : isinf(V.z) ? glm::sign(V.z) : V.z;
    }

    void resolve_collision(const AABB& hitbox)
    {
        glm::vec3 dir = (player.m.bounding_box.get_center() - hitbox.get_center()) / hitbox.get_extent();
        clampnan(dir);
        dir = glm::trunc(dir/maxcomp(dir));
        //player.m.vel = glm::reflect(player.m.vel, dir);
        player.m.vel *= (1.0f-glm::abs(dir));
        player.m.pos = player.m.previousPos + player.m.vel * dt;
        SDL_assert(!glm::any(glm::isnan(player.m.pos)));
        player.m.realVel = (player.m.pos - player.m.previousPos) / dt;
        player.make_AABB();
    }

    void test_collision()
    {
        if (glm::length(player.m.realVel) == 0){
            return;
        }
        player.m.collision = false;
        std::mutex mutex;
        //TODO space partitioning
        std::for_each(std::execution::par, gameScene->Meshes.begin(), gameScene->Meshes.end(),
        [this, &mutex](const mesh& M)
        {
            if (!M.collision)
                return;
            for(const glm::mat4& transform : M.transforms){
                AABB hitbox = M.bounding_box * transform;
                bool collision = 
                player.m.bounding_box.test_AABB(hitbox); //maybe vulnerable to race condition?
                if (collision){
                    mutex.lock();
                    player.m.collision = true;
                    resolve_collision(hitbox);
                    mutex.unlock();
                }
            }
        }
        );
    }

    void update_gameLogic()
    {
        process_events();
        if(escape) {
            show_config();
            return;
        }
        player.move(dt);
        test_collision();
        player.makeView();
        std::for_each(std::execution::par, 
        gameScene->Meshes.begin(), 
        gameScene->Meshes.end(), 
        [this](mesh& M){M.onFrustum(player.m.view_frustum);});
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
    MyGame App(500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}