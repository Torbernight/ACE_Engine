#include <GL/glew.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <execution>
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "input/input.hpp"
#include "physics/physics.hpp"
#include "debug/debug.hpp"
#include "renderer/material.hpp"

#include "game.hpp"

#define XYZW(V) V.x, V.y, V.z, V.w
#define XYZ(V) V.x, V.y, V.z
#define XY(V) V.x, V.y


Game::Game(const std::string& title, int window_w, int window_h)
: win(title, window_w, window_h, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED)
{
}
Game::Game(const std::string& title, int window_w, int window_h, int flags)
: win(title, window_w, window_h, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, flags)
{
}
Game::Game(const std::string& title, int window_w, int window_h, int window_posx, int window_posy, int flags)
: win(title, window_w, window_h, window_posx, window_posy, flags)
{
}

Game::Game(const std::string& title, int window_w, int window_h, int window_posx, int window_posy)
: win(title, window_w, window_h, window_posx, window_posy)
{
}

void Game::processEvents()
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

void Game::initImGui()
{
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(this->win.getHandle(), this->win.getContext());
    ImGui_ImplOpenGL3_Init("#version 460");
    imgui_IO = &ImGui::GetIO();
}

void Game::imguiFrameStart()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Game::imguiFrameEnd()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Game::infoWindow()
{
    if (!showInfo)
        return;
    
    ImGui::Begin("Debug Info", &showInfo);
    ImGui::Text("Frame: %lu", frame);   
    ImGui::Text("Runtime: %fs", runtime);
    ImGui::Text("%f ms", (1.0/fps)*1e+3);
    ImGui::Text("%.2f fps", fps);
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

inline static const char* get_MSAA_str(Game::MSAALevel L)
{
    switch(L)
    {
        case Game::MSAALevel::MS_OFF:
            return "Off";
        case Game::MSAALevel::MS_2:
            return "x2";
        case Game::MSAALevel::MS_4:
            return "x4";
        case Game::MSAALevel::MS_8:
            return "x8";
        default:
            return "";
    }
}


void Game::showConfig()
{
    ImGui::Begin("Config", &open_menu);

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
    postProc_Update |= ImGui::Checkbox("FXAA", &gameScene->postprocess.FXAA);

    ImGui::SeparatorText("Graphics Options");

    int msaa_level = (int)MSAA;
    if (ImGui::SliderInt("MSAA", &msaa_level, 0, MSAALevel::MS_NUM_OPTIONS-1, get_MSAA_str(MSAA), ImGuiSliderFlags_AlwaysClamp))
    {
        MSAA = (MSAALevel)(msaa_level);
        msaa_level = MSAA == MS_OFF ? 0 : 1 << msaa_level;
        gameScene->get_DRenderer().reset(msaa_level);
        if (msaa_level){
            std::string MSAA_LEVEL_STR = "#define MSAA_LEVEL " + std::to_string(msaa_level) + "\n";
            gameScene->reload_DeferredShader(Renderer::Fragment, {MSAA_LEVEL_STR});
        } else {
            gameScene->reload_DeferredShader(Renderer::Fragment);
        }
    }

    uint8_t camera_update = 0;
    ImGui::SeparatorText("Camera options");
    camera_update |= ImGui::DragFloat("near plane", &player.m.zNear);
    camera_update |= ImGui::DragFloat("far plane", &player.m.zFar);
    camera_update |= ImGui::DragFloat("FOV", &player.m.FOVy, 0.01f, M_PIf/16.0f, M_PIf/2.0f);

    ImGui::SeparatorText("Scene Info");

    for (const auto &M : gameScene->Meshes) {
        ImGui::Text("Mesh name: %s", M.m.name.c_str());
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


void Game::initGLParameters()
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
void Game::initAudio()
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

void Game::initSDLParameters()
{
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GL_SetSwapInterval(vsync);
}

void Game::init()
{
    glewExperimental = GL_TRUE;
    int status = glewInit();
    if (status != GLEW_OK){
        std::cerr << "Fatal error: couldn't initialize GLEW library:\n\t" << glewGetErrorString(status) << std::endl;
    }
    glClearColor(0.3, 0.5, 0.4, 1.0);

    initSDLParameters();
    initGLParameters();
    initImGui();
    initAudio();

    gameScene = Scene::import_scene("assets/garden.gltf");
    if (!gameScene){
        win.set_shouldClose(true);
        return;
    }
    gameScene->load_SceneShader("shader/scene.vert.glsl", "shader/scene.frag.glsl");
    gameScene->load_DeferredShader("shader/deferred.vert.glsl", "shader/deferred.frag.glsl");
    gameScene->load_PostProcShader("shader/deferred.vert.glsl", "shader/postprocess.frag.glsl");
    gameScene->initSkybox("shader/skybox.vert.glsl", "shader/skybox.frag.glsl", "ps.tif");
    gameScene->setUBO();
    gameScene->initMeshes();
    gameScene->initDRenderer(win.getWidth(), win.getHeight(), MSAA);

    if (!glm::any(glm::isnan(gameScene->get_defaultCamPos())))
        player.setPos(gameScene->get_defaultCamPos());
    
    player.makeView();
    player.setAspect(win.getWidth() / (float)win.getHeight());
    player.makeProj();

    TransformBuffer.generate();
    TransformBuffer.bufferData((2+gameScene->getMaxInstances())*sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
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

void Game::processWindowEvents(const SDL_WindowEvent &E)
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

void Game::resizeWindow(int w, int h)
{
    win.set_size(w, h);
    gameScene->resizeViewport(w,h);
    glViewport(0,0,w,h);
    float aspect = w / (float)h;
    player.setAspect(aspect);
    player.makeProj();
    TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
}

void Game::setVSync()
{
    if(SDL_GL_SetSwapInterval(vsync ? -1 : 0))
        SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

void Game::updateUniformBuffers()
{
    CameraUBO camBuff = {glm::vec4(player.getPos(), 0), glm::vec4(player.getFront(), 0), player.getNear(), player.getFar()};
    Camera_Buffer.bufferSubData(sizeof(camBuff), 0, &camBuff);
    TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView());
    StateUBO stateBuff = {frame, runtime, dt, {win.getWidth(), win.getHeight()}};
    State_Buffer.bufferSubData(sizeof(stateBuff), 0, &stateBuff);
}

void Game::testCollision()
{
    test_player_collision(player, gameScene, tick_rate);
    test_object_collision(gameScene, tick_rate);
}

bool Game::meshIntersects(const Renderer::Mesh& M)
{
    glm::vec2 intersection = (M.m.bounding_box*M.m.transform).ray_intersection(player.getPos(), player.getFront());
    return intersection.x <= intersection.y && intersection.x <= 3.0f;
}

DynamicObject* Game::selectObject()
{
    float dist = FLT_MAX;
    float closest_static = FLT_MAX;
    DynamicObject *M = nullptr;
    for(Object *Obj : gameScene->objects){
        if (Obj->m_grabbed)
            continue;
        Renderer::Mesh &mesh = gameScene->Meshes[Obj->m.mesh];
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
    auto M = std::find_if(gameScene->dynamic_objects.begin(), gameScene->dynamic_objects.end(), 
    [this, &dist](const dynamic_object& Obj)
    {
        if (Obj.m_grabbed)
            return false;
        auto &M = gameScene->Meshes[Obj.m.mesh];
        float d = glm::distance(player.m.pos, (M.m.bounding_box*M.m.transform).get_center());
        bool closest = d < dist;
        dist = closest ? d : dist;
        return meshIntersects(M) && closest;
    });    for(auto &o : gameScene->dynamic_objects)
    return M == gameScene->dynamic_objects.end() ? nullptr : &(*M);
    */
}

void Game::updatePhysics()
{
    for (auto &Obj : gameScene->dynamic_objects) 
        Obj.update(&player, gameScene, tick_rate);

    testCollision();

    if (player.has_grabbed_object())
        player.m.grabbed_object->update_grabbed(&player, gameScene);

}

void Game::pointObject()
{
    DynamicObject* object = player.m.pointed_object;

    if (object)
        return;

    if ((object = selectObject()))
        object->m_selected = true;

    player.m.pointed_object = object;
}

void Game::useObject()
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

void Game::checkSelectableObject()
{
    InputHandler& input = InputHandler::get();
    pointObject();
    if (input.is_key_pressed('e'))
        useObject();
}

void Game::frustumTest()
{
    std::for_each(std::execution::par, gameScene->Meshes.begin(), gameScene->Meshes.end(), [this](Renderer::Mesh& M){M.onFrustum(player.m.view_frustum);});
}

void Game::updateGameLogic()
{
    static float accumulated = 0.0f;
    accumulated += dt;
    InputHandler &input = InputHandler::get();

    while (accumulated >= tick_rate)
    {
        player.reset_rotation();
        processEvents();

        if (input.is_key_just_released(SDLK_ESCAPE)) {
            open_menu = !open_menu;
            SDL_SetRelativeMouseMode(open_menu ? SDL_TRUE : SDL_FALSE);
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
        fps = 1.0/dt;
    }

    player.makeView();
    frustumTest();
}

void Game::startFrame()
{
    frame_timer.Start();
    imguiFrameStart();
}

void Game::endFrame()
{
    imguiFrameEnd();
    win.swapBuffer();
    frame++;
    runtime += (dt = frame_timer.Count());
}

void Game::end_norender()
{
    runtime += (dt = frame_timer.Count());
}

void Game::mainLoop()
{
    while(!win.shouldClose())
    {
        startFrame();

        updateGameLogic();

        if(open_menu)
            showConfig();

        infoWindow();
        
        if (pause_rendering) {
            end_norender();
            continue;
        }

        updateUniformBuffers();

        gameScene->render(TransformBuffer, 2*sizeof(glm::mat4));

        endFrame();
    }
}

void Game::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

int main()
{
    Game App("ACE Engine", 500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}