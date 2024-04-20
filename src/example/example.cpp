#include <iostream>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <execution>
#include <glm/gtc/quaternion.hpp>

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
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    void init()
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_GL_SetSwapInterval(0);
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
        player.m.pos = player.m.previousPos + player.m.vel * tick_rate;
        SDL_assert(!glm::any(glm::isnan(player.m.pos)));
        player.m.realVel = (player.m.pos - player.m.previousPos) / tick_rate;
        player.make_AABB();
    }

    void resolve_collision(const AABB& box, mesh& M, const AABB& hitbox)
    {
        glm::vec3 dir = (box.get_center() - hitbox.get_center()) / hitbox.get_extent();
        clampnan(dir);
        dir = glm::trunc(dir/maxcomp(dir));
        glm::vec3 Pos = glm::vec3(M.transforms[0][3]);
        glm::vec3 previousPos = Pos - M.vel*tick_rate;
        M.vel *= (1.0f-glm::abs(dir));
        Pos = previousPos + M.vel * tick_rate;
        SDL_assert(!glm::any(glm::isnan(Pos)) && !glm::any(glm::isinf(Pos)));
        M.transforms[0][3] = glm::vec4(Pos, 1.0f);
    }

    void test_player_collision()
    {
        if (glm::length(player.m.realVel) == 0){
            return;
        }
        std::mutex mutex;
        player.m.collision = false;
        //TODO space partitioning
        std::for_each(std::execution::par, gameScene->Meshes.begin(), gameScene->Meshes.end(),
        [this, &mutex](const mesh& M)
        {
            if (!M.collision || M.grabbable)
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

    void test_object_collision()
    {
        std::mutex mutex;
        for (auto &M : gameScene->Meshes)
        {
            if (M.static_mesh || M.grabbed)
                continue;
            AABB box = M.bounding_box * M.transforms[0];
            std::for_each(std::execution::par, gameScene->Meshes.begin(), gameScene->Meshes.end(),
            [this, &mutex, &box, &M](const mesh& Mesh)
            {
                if (!Mesh.static_mesh || Mesh.grabbed || Mesh.grabbable)
                    return;
                for(const glm::mat4& transform : Mesh.transforms){
                    AABB hitbox = Mesh.bounding_box * transform;
                    bool collision =  box.test_AABB(hitbox);
                    if (collision){
                        mutex.lock();
                        resolve_collision(box, M, hitbox);
                        box = M.bounding_box * M.transforms[0];
                        mutex.unlock();
                    }
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

    //https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
    // compute the near and far intersections of the cube (stored in the x and y components) using the slab method
    // no intersection means vec.x > vec.y (really tNear > tFar)
    glm::vec2 intersectAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const AABB& box) {
        glm::vec3 tMin = (box.min - rayOrigin) / rayDir;
        glm::vec3 tMax = (box.max - rayOrigin) / rayDir;
        glm::vec3 t1 = glm::min(tMin, tMax);
        glm::vec3 t2 = glm::max(tMin, tMax);
        float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
        float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
        return glm::vec2(tNear, tFar);
    };

    bool meshIntersects(const mesh& M, unsigned int instance = 0)
    {
        glm::vec2 intersection = intersectAABB(player.m.pos, player.m.front, M.bounding_box*M.transforms[instance]);
        return intersection.x <= intersection.y;
    }

    mesh* grab_mesh()
    {
        float dist = FLT_MAX;
        auto M = std::find_if(gameScene->Meshes.begin(), gameScene->Meshes.end(), 
        [this, &dist](const mesh& Mesh)
        {
            if (Mesh.grabbed || Mesh.static_mesh || !Mesh.grabbable)
                return false;
            float d = glm::distance(player.m.pos, (Mesh.bounding_box*Mesh.transforms[0]).get_center());
            bool closest = d < dist;
            //I don't think this works the way I intend it to
            dist = closest ? d : dist;
            return meshIntersects(Mesh) && closest && d <= 3.0f;
        });
        return M == gameScene->Meshes.end() ? nullptr : &(*M);
    }

    static glm::mat4 transform_from_basis(const glm::vec3& R, const glm::vec3& U, const glm::vec3& F, const glm::vec3& P)
    {
        return glm::mat4(
            R.x, R.y, R.z, 0.0f,
            U.x, U.y, U.z, 0.0f,
            F.x, F.y, F.z, 0.0f,
            P.x, P.y, P.z, 1.0f
        );
    }

    void update_physics()
    {
        for (auto &M : gameScene->Meshes) 
        {
            M.lastFrame_Transform = M.transforms[0];

            if (M.static_mesh || M.grabbed)
                continue;

            //M.acc = glm::vec3(0.0f, -9.8f, 0.0f);
            //M.vel += glm::vec3(0.0f, -9.8f*tick_rate, 0.0f);
            //M.transforms[0][3] += glm::vec4(M.vel*tick_rate, 0.0f);
            M.vel.y -= 9.8f*tick_rate;
            M.transforms[0][3].y += M.vel.y*tick_rate;
        }

        test_collision();
        
        if (player.m.grabbed_object)
        {
            glm::vec3 pos = player.m.pos - player.m.up*0.125f + player.m.front*0.6f + player.m.right*0.1f;
            glm::mat4 trans = transform_from_basis(player.m.right, player.m.up, -player.m.front, pos);
            player.m.grabbed_object->transforms[0] = trans;
        }
    }

    void point_object()
    {
        mesh* object = player.m.pointed_object;

        if (object)
            object->selected = false;

        if ((object = grab_mesh()))
            object->selected = true;

        player.m.pointed_object = object;
    }

    void grab_object() //a.k.a press use while pointing a selectable object
    {
        mesh* grabbed = player.m.grabbed_object; 
        if (!grabbed)
        {
            if ((grabbed = player.m.pointed_object)){
                grabbed->grabbed = true;
                grabbed->selected = false;
                player.m.grabbed_object = grabbed;
            }
            return;
        }
        grabbed->grabbed = false;
        player.m.grabbed_object = nullptr;
    }

    void check_selectable_object()
    {
        point_object();
        if (player.movement.use)
            grab_object();
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

    void update_gameLogic()
    {
        static float accumulated = 0.0f;
        accumulated += dt;
        glm::mat4 lastFrame_Transform = get_camera_transform();

        while (accumulated >= tick_rate)
        {
            lastFrame_Transform = get_camera_transform();
            player.m.delta_rotation = glm::vec2(0.0f);
            process_events();
            if (escape){
                accumulated -= tick_rate;
                continue;
            }
            check_selectable_object();
            player.move(tick_rate);
            player.rotate(tick_rate);
            update_physics();
            accumulated -= tick_rate;
        }
        
        float factor = accumulated/tick_rate;
        for (auto& M : gameScene->Meshes) {
            if (!M.grabbable || M.static_mesh) {
                continue;
            }
            M.transforms[0] = interpolate_transform(M.lastFrame_Transform, M.transforms[0], factor);
        }
        glm::mat4 current_Transform = get_camera_transform();
        glm::mat4 interpolated_transform = interpolate_transform(lastFrame_Transform, current_Transform, factor);
        set_camera_transform(interpolated_transform);
        //std::sort(gameScene->Meshes.begin(), gameScene->Meshes.end(), 
        //[this](const mesh& A, const mesh& B){
        //    return 
        //    glm::distance(player.m.pos, (A.bounding_box*A.transforms[0]).get_center()) 
        //    < 
        //    glm::distance(player.m.pos, (B.bounding_box*B.transforms[0]).get_center());
        //    });
        
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

            if(escape) {
                show_config();
            }

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