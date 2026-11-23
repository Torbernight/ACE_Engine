#include <glad/gl.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <execution>

#include "SDL_keycode.h"
#include "SDL_mouse.h"

#include "SDL_video.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/imgui.h"

#include "../input/input.hpp"
#include "../physics/physics.hpp"
#include "../debug/debug.hpp"
#include "../renderer/material.hpp"
#include "settings.hpp"

#include "engine.hpp"

#define XYZW(V) V.x, V.y, V.z, V.w
#define XYZ(V) V.x, V.y, V.z
#define XY(V) V.x, V.y


Engine::Engine(const std::string& title, int window_w, int window_h)
: win(title, window_w, window_h, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED)
{
}
Engine::Engine(const std::string& title, int window_w, int window_h, int flags)
: win(title, window_w, window_h, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, flags)
{
}
Engine::Engine(const std::string& title, int window_w, int window_h, int window_posx, int window_posy, int flags)
: win(title, window_w, window_h, window_posx, window_posy, flags)
{
}

Engine::Engine(const std::string& title, int window_w, int window_h, int window_posx, int window_posy)
: win(title, window_w, window_h, window_posx, window_posy)
{
}

void Engine::processEvents()
{
    InputHandler &input = InputHandler::get();
    
    input.begin_new_frame();

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
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
            case SDL_MOUSEMOTION:
                if (imgui_IO->WantCaptureMouse){
                    ImGui_ImplSDL2_ProcessEvent(&E);
                } else {
                    input.process_mouse_event(E);
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (imgui_IO->WantCaptureKeyboard){
                    ImGui_ImplSDL2_ProcessEvent(&E);
                } else {
                    input.process_keyboard_event(E.key);
                }
                break;
            case SDL_TEXTINPUT:
                if (imgui_IO->WantCaptureKeyboard){
                    ImGui_ImplSDL2_ProcessEvent(&E);
                }
                break;
        }
    }
}

void Engine::initImGui()
{
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(this->win.getHandle(), this->win.getContext());
    ImGui_ImplOpenGL3_Init("#version 460");
    imgui_IO = &ImGui::GetIO();
}

void Engine::imguiFrameStart()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Engine::imguiFrameEnd()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Engine::infoWindow()
{
    SettingsManager& settings = SettingsManager::get();
    bool& showInfo = settings["show info"].as_bool();
    ImGui::Begin("Debug Info", &showInfo);
    ImGui::Text("Frame: %lu", frame);   
    ImGui::Text("Runtime: %fs", runtime);
    ImGui::Text("%f ms", timeDelta*1e+3);
    ImGui::Text("%.2f fps", 1.0f/timeDelta);
    ImGui::Text("position: %f %f %f", XYZ(player.m.pos));
    ImGui::Text("speed: %f %f %f", XYZ(player.m.realVel));
    ImGui::Text("angle: %f %f", XY(glm::degrees(player.m.rotation)));
    ImGui::Text("delta angle: %f %f", XY(glm::degrees(player.m.delta_rotation)));
    ImGui::Text("on floor: %s",player.m.floor_collision ? "true" : "false");
    //ImGui::Text("footstep timer: %f",player.m.footstep_timer);
    //ImGui::Text("footstep amp: %f",player.m.footstep_amp);
    ImGui::Text("collision: %s",player.m.collision ? "true" : "false");
    ImGui::End();
}

inline static const char* get_MSAA_str(unsigned int L)
{
    switch(L)
    {
        case 0:
            return "Off";
        case 1:
            return "x2";
        case 2:
            return "x4";
        case 3:
            return "x8";
        default:
            return "";
    }
}


void Engine::showConfig()
{
    SettingsManager &settings = SettingsManager::get();
    ImGui::Begin("Config", &open_menu);

    bool any_setting_changed            = false;
    bool post_process_settings_changed  = false;
    bool camera_settings_changed        = false;
    bool vsync_changed                  = false;
    bool mouse_sensitivity_changed      = false;
    bool msaa_changed                   = false;

    bool  &vsync             = settings["vsync"]            .as_bool();
    bool  &showInfo          = settings["show info"]        .as_bool();
    bool  &FXAA              = settings["FXAA"]             .as_bool();
    int   &MSAA              = settings["MSAA"]             .as_int();
    float &mouse_sensitivity = settings["mouse sensitivity"].as_float();
    float &Brightness        = settings["brightness"]       .as_float();
    float &Contrast          = settings["contrast"]         .as_float();
    float &Saturation        = settings["saturation"]       .as_float();
    float &Gamma             = settings["gamma"]            .as_float();
    float &Exposure          = settings["exposure"]         .as_float();
    float &far_plane         = settings["far plane"]        .as_float();
    float &near_plane        = settings["near plane"]       .as_float();
    float &FOV               = settings["FOV"]              .as_float();

    ImGui::Checkbox("Show Info", &showInfo);

    vsync_changed = ImGui::Checkbox("VSync", &vsync);

    ImGui::SeparatorText("Controls");
    mouse_sensitivity_changed = ImGui::DragFloat("Mouse Sensitivity", &mouse_sensitivity, 1.0f, 1.0f, 100.0f);

    ImGui::SeparatorText("Post Processing");
    if(ImGui::DragFloat("Brightness", &Brightness, 0.025, -1.0, 2.0))  post_process_settings_changed = true;
    if(ImGui::DragFloat("Contrast", &Contrast, 0.025, 0.0, 2.0))       post_process_settings_changed = true;
    if(ImGui::DragFloat("Saturation", &Saturation, 0.025, 0.0, 4.0))   post_process_settings_changed = true;
    if(ImGui::DragFloat("Gamma", &Gamma, 0.025, 0.0, 4.0))             post_process_settings_changed = true;
    if(ImGui::DragFloat("Exposure", &Exposure, 0.1, -5.0, 5.0))        post_process_settings_changed = true;
    if(ImGui::Checkbox("FXAA", &FXAA))                                 post_process_settings_changed = true;

    ImGui::SeparatorText("Graphics Options");
    msaa_changed = ImGui::SliderInt("MSAA", &MSAA, 0, 4, get_MSAA_str(MSAA), ImGuiSliderFlags_AlwaysClamp);

    ImGui::SeparatorText("Camera options");
    if (ImGui::DragFloat("near plane", &near_plane))                    camera_settings_changed = true;
    if (ImGui::DragFloat("far plane", &far_plane))                      camera_settings_changed = true;
    if (ImGui::DragFloat("FOV", &FOV, 0.01f, M_PIf/16.0f, M_PIf/2.0f))  camera_settings_changed = true;

    any_setting_changed = camera_settings_changed || mouse_sensitivity_changed || msaa_changed || vsync_changed || post_process_settings_changed;

    ImGui::SeparatorText("Scene Info");

    for (const auto &M : currentScene->Meshes) {
        ImGui::Text("Mesh name: %s", M.m.name.c_str());
        ImGui::Text("visible: %u", M.m.visible);
        ImGui::Separator();
    }

    if (post_process_settings_changed){
        currentScene->updatePostProcessBuffer();
    }
    if (camera_settings_changed){
        player.setFOV(FOV);
        player.setFarPlane(far_plane);
        player.setNearPlane(near_plane);
        TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
    }
    if (vsync_changed){
        setVSync();
    }
    if (msaa_changed)
    {
        int msaa_level = MSAA == 0 ? 0 : 1 << MSAA;
        currentScene->get_DRenderer().reset(msaa_level);
        if (msaa_level){
            std::string MSAA_LEVEL_STR = "#define MSAA_LEVEL " + std::to_string(msaa_level) + "\n";
            currentScene->reload_DeferredShader(Renderer::ShaderStage_Fragment, {MSAA_LEVEL_STR});
        } else {
            currentScene->reload_DeferredShader(Renderer::ShaderStage_Fragment);
        }
    }
    if (any_setting_changed)
    {
        settings.save("settings.yaml");
    }

    ImGui::End();
}


void Engine::initGLParameters()
{
#   ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, nullptr);
#   endif
    glEnable(GL_PRIMITIVE_RESTART);
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void Engine::initAudio()
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
}

void Engine::initSDLParameters()
{
    SettingsManager &settings = SettingsManager::get();
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GL_SetSwapInterval(settings["vsync"].as_bool());
}

void Engine::init()
{
    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    printf("loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glClearColor(0.3, 0.5, 0.4, 1.0);

    SettingsManager &settings = SettingsManager::get();
    settings.load("settings.yaml");

    initSDLParameters();
    initGLParameters();
    initImGui();
    initAudio();

    currentScene = Scene::import_scene("assets/garden.gltf");
    if (!currentScene){
        win.set_shouldClose(true);
        return;
    }
    currentScene->load_SceneShader("shader/scene.vert.glsl", "shader/scene.frag.glsl");
    currentScene->load_DeferredShader("shader/deferred.vert.glsl", "shader/deferred.frag.glsl");
    currentScene->load_PostProcShader("shader/deferred.vert.glsl", "shader/postprocess.frag.glsl");
    currentScene->initSkybox("shader/skybox.vert.glsl", "shader/skybox.frag.glsl", "ps.tif");
    currentScene->setUBO();
    currentScene->initMeshes();
    currentScene->initDRenderer(win.getWidth(), win.getHeight(), settings["MSAA"].as_uint());

    if (!glm::any(glm::isnan(currentScene->get_defaultCamPos())))
        player.setPos(currentScene->get_defaultCamPos());
    
    player.makeView();
    player.setAspect(win.getWidth() / (float)win.getHeight());
    player.makeProj();

    TransformBuffer.generate();
    TransformBuffer.bufferData((2+currentScene->getMaxInstances())*sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    TransformBuffer.bindBufferBase(0);
    TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
    TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView()[0][0]);
    Camera_Buffer.generate();
    Camera_Buffer.bufferData(sizeof(CameraUBO), nullptr, GL_DYNAMIC_DRAW);
    Camera_Buffer.bindBufferBase(1);
    StateUBO buff = {};
    State_Buffer.generate();
    State_Buffer.bufferData(sizeof(StateUBO), &buff, GL_DYNAMIC_DRAW);
    State_Buffer.bindBufferBase(5);
}

void Engine::processWindowEvents(const SDL_WindowEvent &E)
{
    switch(E.event)
    {
        case SDL_WINDOWEVENT_RESIZED:
            resizeWindow(E.data1, E.data2);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            pause_rendering = true;
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            pause_rendering = false;
            break;
    }
}

void Engine::resizeWindow(int w, int h)
{
    win.set_size(w, h);
    currentScene->resizeViewport(w,h);
    glViewport(0,0,w,h);
    float aspect = w / (float)h;
    player.setAspect(aspect);
    player.makeProj();
    TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
}

void Engine::setVSync()
{
    SettingsManager &settings = SettingsManager::get();
    bool vsync = settings["vsync"].as_bool();
    if(SDL_GL_SetSwapInterval(vsync ? -1 : 0))
        SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

void Engine::updateUniformBuffers()
{
    CameraUBO camBuff = {glm::vec4(player.getPos(), 0), glm::vec4(player.getFront(), 0), player.getNear(), player.getFar()};
    Camera_Buffer.bufferSubData(sizeof(camBuff), 0, &camBuff);
    TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView());
    StateUBO stateBuff = {frame, runtime, timeDelta, {win.getWidth(), win.getHeight()}};
    State_Buffer.bufferSubData(sizeof(stateBuff), 0, &stateBuff);
}

void Engine::testCollision()
{
    test_player_collision(player, currentScene, tick_rate);
    test_object_collision(currentScene, tick_rate);
}

bool Engine::meshIntersects(const Renderer::Mesh& M)
{
    glm::vec2 intersection = (M.m.bounding_box*M.m.transform).ray_intersection(player.getPos(), player.getFront());
    return intersection.x <= intersection.y && intersection.x <= 3.0f;
}

DynamicObject* Engine::selectObject()
{
    float dist = FLT_MAX;
    float closest_static = FLT_MAX;
    DynamicObject *M = nullptr;
    for(Object *Obj : currentScene->objects){
        if (Obj->m_grabbed)
            continue;
        Renderer::Mesh &mesh = currentScene->Meshes[Obj->m.mesh];
        float d = glm::distance(player.m.pos, (mesh.m.bounding_box*mesh.m.transform).get_center());
        
        if(dynamic_cast<DynamicObject*>(Obj) == nullptr){
            closest_static = d < closest_static ? d : closest_static; 
            continue;
        }

        bool closest = d < dist;
        dist = closest ? d : dist;
        if (closest){
            M = dynamic_cast<DynamicObject*>(Obj);
        }
    }
    return closest_static < dist ? nullptr : M;
    /*
    auto M = std::find_if(currentScene->dynamic_objects.begin(), currentScene->dynamic_objects.end(), 
    [this, &dist](const dynamic_object& Obj)
    {
        if (Obj.m_grabbed)
            return false;
        auto &M = currentScene->Meshes[Obj.m.mesh];
        float d = glm::distance(player.m.pos, (M.m.bounding_box*M.m.transform).get_center());
        bool closest = d < dist;
        dist = closest ? d : dist;
        return meshIntersects(M) && closest;
    });    for(auto &o : currentScene->dynamic_objects)
    return M == currentScene->dynamic_objects.end() ? nullptr : &(*M);
    */
}

void Engine::updatePhysics()
{
    for (auto &Obj : currentScene->dynamic_objects) 
        Obj.update(&player, currentScene, tick_rate);

    testCollision();

    if (player.has_grabbed_object())
        player.m.grabbed_object->update_grabbed(&player, currentScene);

}

void Engine::pointObject()
{
    DynamicObject* object = player.m.pointed_object;

    if (object)
        return;

    if ((object = selectObject()))
        object->m_selected = true;

    player.m.pointed_object = object;
}

void Engine::useObject()
{
    DynamicObject* grabbed = player.m.grabbed_object; 
    if (!grabbed){
        if ((grabbed = player.m.pointed_object)){
            grabbed->grab(&player);
        }
        return;
    }
    grabbed->use(&player);
}

void Engine::checkSelectableObject()
{
    InputHandler& input = InputHandler::get();
    pointObject();
    if (input.is_key_pressed('e'))
        useObject();
}

void Engine::frustumTest()
{
    std::for_each(std::execution::par, currentScene->Meshes.begin(), currentScene->Meshes.end(), [this](Renderer::Mesh& M){M.onFrustum(player.m.view_frustum);});
}

void Engine::updateGameLogic()
{
    static float accumulated = 0.0f;
    InputHandler &input = InputHandler::get();

    while (accumulated >= tick_rate)
    {
        player.reset_rotation();
        processEvents();

        if (input.is_key_just_released(SDLK_ESCAPE)) {
            open_menu = !open_menu;
            SDL_SetRelativeMouseMode(open_menu ? SDL_FALSE : SDL_TRUE);
        }
        
        if (!open_menu)
        {
            checkSelectableObject();
            player.move(tick_rate);
            player.rotate(tick_rate);
            updatePhysics();
            if (player.should_sound_footstep() && !Mix_Playing(0))
                Mix_PlayChannel(0, footsteps_wave, 0);
        }

        accumulated -= tick_rate;
        ticks++;
    }

    player.makeView();
    frustumTest();
}

void Engine::startFrame()
{
    frame_timer.Start();
    imguiFrameStart();
}

void Engine::endFrame()
{
    imguiFrameEnd();
    win.swapBuffer();
    frame++;
}

void Engine::endFrameWithoutRendering()
{
}

void Engine::mainLoop()
{
    SettingsManager &settings = SettingsManager::get();
    while(!win.shouldClose())
    {
        startFrame();

        updateGameLogic();

        if(open_menu){
            showConfig();
        }

        if (settings["show info"].as_bool()){
            infoWindow();
        }
        
        if (pause_rendering) {
            endFrameWithoutRendering();
            continue;
        }

        updateUniformBuffers();

        currentScene->render(TransformBuffer, 2*sizeof(glm::mat4));

        endFrame();
    }
}

void Engine::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

int main()
{
    Engine App("ACE Engine", 500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}