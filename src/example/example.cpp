#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <execution>
#include <glm/gtc/quaternion.hpp>

#include "../application.hpp"
#include "../vertexArray.hpp"
#include "../buffer.hpp"

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
        gameScene->load_DeferredShader("shader/deferred.vert.spv", "shader/deferred.frag.spv");
        gameScene->load_PostProcShader("shader/deferred.vert.spv", "shader/postprocess.frag.spv");
        gameScene->initSkybox("shader/skybox.vert.spv", "shader/skybox.frag.spv");
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
        
        ImGui::Begin("Info", &showInfo);
        ImGui::Text("Frame: %lu", frame);   
        ImGui::Text("Runtime: %fs", runtime);
        ImGui::Text("%f ms", (1.0/fps)*1e+3);
        ImGui::Text("%.2f fps", fps);
        ImGui::Text("position: %f %f %f", XYZ(player.m.pos));
        ImGui::Text("speed: %f %f %f", XYZ(player.m.realVel));
        ImGui::Text("angle: %f %f", XY(glm::degrees(player.m.rotation)));
        ImGui::Text("delta angle: %f %f", XY(glm::degrees(player.m.delta_rotation)));
        ImGui::Text("on floor: %s",player.m.floor_collision ? "true" : "false");
        ImGui::Text("footstep timer: %f",player.m.footstep_timer);
        ImGui::Text("footstep amp: %f",player.m.footstep_amp);
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
        player.m.floor_collision = dir.y == 1.0;
        //player.m.vel = glm::reflect(player.m.vel, dir);
        player.m.vel *= (1.0f-glm::abs(dir));
        player.m.pos = player.m.previousPos + player.m.vel * tick_rate;
        //SDL_assert(!glm::any(glm::isnan(player.m.pos)));
        player.m.realVel = (player.m.pos - player.m.previousPos) / tick_rate;
        player.make_AABB();
    }

    void resolve_collision(const AABB& box, dynamic_object& Obj, const AABB& hitbox)
    {
        auto &M = gameScene->Meshes[Obj.m.Mesh];
        glm::vec3 dir = (box.get_center() - hitbox.get_center()) / hitbox.get_extent();
        clampnan(dir);
        dir = glm::trunc(dir/maxcomp(dir));
        glm::vec3 Pos = glm::vec3(M.m.transform[3]);
        glm::vec3 previousPos = Pos - Obj.vel*tick_rate;
        Obj.vel *= (1.0f-glm::abs(dir));
        Pos = previousPos + Obj.vel * tick_rate;
        //SDL_assert(!glm::any(glm::isnan(Pos)) && !glm::any(glm::isinf(Pos)));
        M.m.transform[3] = glm::vec4(Pos, 1.0f);
    }

    void test_narrow_collision(const static_object& obj)
    {
        using namespace glm;
        AABB box = player.m.bounding_box;
        for (const auto &tri : obj.collision_mesh)
        {
            vec3 normal;
            if (tri.test_AABB(box, normal))
            {
                if (normal == vec3(0))
                    continue;
                
                bool is_floor = normal.y > 0.1;
                bool is_wall = !is_floor && std::abs(normal.y) <= 0.1; 
                if (player.m.jump && is_floor){
                    continue;
                }
                player.m.floor_collision = is_floor;
                /*
                vec3 ro = player.m.previousPos-vec3(0,1.7f,0), rd = normalize(player.m.vel);
                float t = plane(normal, tri.A).ray_intersection(
                    ro, rd);
                vec3 c = ro+rd*t+vec3(0,1.7,0);
                if (any(isinf(c)) || any(isnan(c)))
                    continue;
                vec3 p0 = c - player.m.previousPos;
                vec3 p1 = player.m.pos - c;
                p1 = reflect(p1, normal);
                player.m.vel = (p0+p1)/tick_rate;
                player.m.pos = player.m.previousPos + p0 + p1;
                */
                vec3 dir = trunc(normal/maxcomp(normal));
                player.m.vel *= (1.0f-abs(dir));

                player.m.realVel = (player.m.pos - player.m.previousPos) / tick_rate;
                player.make_AABB();
                box = player.m.bounding_box;
            }
        }
    }

    void test_player_collision()
    {
        if (glm::length(player.m.realVel) == 0){
            return;
        }
        std::mutex mutex;
        player.m.collision = false;
        player.m.on_floor_last_frame = player.m.floor_collision;
        player.m.floor_collision = false;
        //TODO space partitioning
        std::for_each(std::execution::par, gameScene->static_objects.begin(), gameScene->static_objects.end(),
        [this, &mutex](const static_object& Obj)
        {
            auto &M = gameScene->Meshes[Obj.m.Mesh];
            const glm::mat4& transform = M.m.transform;
            AABB hitbox = M.m.bounding_box * transform;
            if (player.m.bounding_box.test_AABB(hitbox)){
                mutex.lock();
                player.m.collision = true;
                test_narrow_collision(Obj);
                //resolve_collision(hitbox);
                mutex.unlock();
                }
        }
        );
    }

    void test_object_collision()
    {
        std::mutex mutex;
        for (auto &Obj : gameScene->dynamic_objects)
        {
            if (Obj.m_grabbed)
                continue;
            auto &M = gameScene->Meshes[Obj.m.Mesh];
            AABB box = M.m.bounding_box * M.m.transform;
            auto begin = gameScene->static_objects.begin();
            auto end = gameScene->static_objects.end();
            std::for_each(std::execution::par,begin,end,
            [this, &mutex, &box, &M, &Obj](const static_object& static_obj)
            {
                auto &Mesh = gameScene->Meshes[static_obj.m.Mesh];
                const glm::mat4& transform = Mesh.m.transform;
                AABB hitbox = Mesh.m.bounding_box * transform;
                if (box.test_AABB(hitbox))
                {
                    mutex.lock();
                    resolve_collision(box, Obj, hitbox);
                    box = M.m.bounding_box * M.m.transform;
                    mutex.unlock();
                }
            }
        );
        }
    }

    void test_collision()
    {
        test_player_collision();
        test_object_collision();
    }

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

    void update_physics()
    {
        for (auto &Obj : gameScene->dynamic_objects) 
            Obj.update(&player, gameScene, tick_rate);

        test_collision();

        if (player.has_grabbed_object())
            player.m.grabbed_object->update_grabbed(&player, gameScene);

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
        if (!grabbed)
        {
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

    glm::mat4 interpolate_transform(const glm::mat4& V0, const glm::mat4& V1, float fac)
    {
        glm::quat R0 = glm::mat3(V0), R1 = glm::mat3(V1);
        glm::quat R = glm::slerp(R0, R1, fac);
        glm::vec4 P = glm::mix(V0[3], V1[3], fac);
        glm::mat4 V(R);
        V[3] = P;
        return V;
    }

    glm::mat4 get_camera_transform()
    {
        return 
        glm::mat4(
            player.m.right.x, player.m.right.y, player.m.right.z, 0.0f,
            player.m.up.x, player.m.up.y, player.m.up.z, 0.0f,
            -player.m.front.x, -player.m.front.y, -player.m.front.z, 0.0f,
            player.m.pos.x, player.m.pos.y, player.m.pos.z, 1.0f
        );
    }

    void set_camera_transform(const glm::mat4& T)
    {
        player.m.right = glm::mat3(T) * glm::vec3(1.0,0.0,0.0);
        player.m.up = glm::mat3(T) * glm::vec3(0.0,1.0,0.0);
        player.m.front = glm::mat3(T) * glm::vec3(0.0,0.0,-1.0);
        player.m.pos = glm::vec3(T[3]);
    }

    void order_objects()
    {
        auto &indices = gameScene->Mesh_indices;
        auto &meshes = gameScene->Meshes;
        indices.clear();

        for(auto it = meshes.begin(); it != meshes.end(); it++){
            if (!it->m.visible)
                continue;
            indices.push_back(it - meshes.begin());
        }

        std::sort(indices.begin(), indices.end(), 
        [&meshes, this](unsigned int A, unsigned int B){
            glm::vec3 P0 = (meshes[A].m.bounding_box*meshes[A].m.transform).get_center();
            glm::vec3 P1 = (meshes[B].m.bounding_box*meshes[A].m.transform).get_center();
            return glm::distance(P0, player.m.pos) <= glm::distance(P1, player.m.pos);
        });
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
                goto skip_loop;
            
            check_selectable_object();
            player.move(tick_rate);
            player.rotate(tick_rate);
            update_physics();
            if (player.should_sound_footstep() && !Mix_Playing(0))
                Mix_PlayChannel(0, footsteps_wave, 0);

skip_loop:
            accumulated -= tick_rate;
            ticks++;
            if (!(ticks%20))
                fps = 1.0/dt;
        }

        player.makeView();
        check_on_frustum();
        //order_objects();
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
    MyGame App(500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}