#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <execution>

#include "example.hpp"

void MyGame::initImGui()
{
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(this->win.getHandle(), this->win.getContext());
    ImGui_ImplOpenGL3_Init("#version 460");
    imgui_IO = &ImGui::GetIO();
}

void MyGame::initGLParameters()
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
void MyGame::initAudio()
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

void MyGame::initSDLParameters()
{
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GL_SetSwapInterval(vsync);
}

void MyGame::init()
{
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

void MyGame::resizeWindow(int w, int h)
{
    win.set_size(w, h);
    gameScene->resizeViewport(w,h);
    glViewport(0,0,w,h);
    float aspect = w / (float)h;
    player.setAspect(aspect);
    player.makeProj();
    TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
    
}

void MyGame::setVSync()
{
    if(SDL_GL_SetSwapInterval(vsync ? -1 : 0))
        SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

void MyGame::updateUniformBuffers()
{
    CameraUBO camBuff = {glm::vec4(player.getPos(), 0), glm::vec4(player.getFront(), 0), player.getNear(), player.getFar()};
    Camera_Buffer.bufferSubData(sizeof(camBuff), 0, &camBuff);
    TransformBuffer.bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), &player.getView());
    StateUBO stateBuff = {frame, runtime, dt, {win.getWidth(), win.getHeight()}};
    State_Buffer.bufferSubData(sizeof(stateBuff), 0, &stateBuff);
}

void MyGame::testCollision()
{
    test_player_collision(player, gameScene, tick_rate);
    test_object_collision(gameScene, tick_rate);
}

bool MyGame::meshIntersects(const Mesh& M)
{
    glm::vec2 intersection = (M.m.bounding_box*M.m.transform).ray_intersection(player.getPos(), player.getFront());
    return intersection.x <= intersection.y && intersection.x <= 3.0f;
}

DynamicObject* MyGame::selectObject()
{
    float dist = FLT_MAX;
    float closest_static = FLT_MAX;
    DynamicObject *M = nullptr;
    for(Object *Obj : gameScene->objects){
        if (Obj->m_grabbed)
            continue;
        Mesh &Mesh = gameScene->Meshes[Obj->m.Mesh];
        float d = glm::distance(player.m.pos, (Mesh.m.bounding_box*Mesh.m.transform).get_center());
        
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
        auto &M = gameScene->Meshes[Obj.m.Mesh];
        float d = glm::distance(player.m.pos, (M.m.bounding_box*M.m.transform).get_center());
        bool closest = d < dist;
        dist = closest ? d : dist;
        return meshIntersects(M) && closest;
    });    for(auto &o : gameScene->dynamic_objects)
    return M == gameScene->dynamic_objects.end() ? nullptr : &(*M);
    */
}

void MyGame::updatePhysics()
{
    for (auto &Obj : gameScene->dynamic_objects) 
        Obj.update(&player, gameScene, tick_rate);

    testCollision();

    if (player.has_grabbed_object())
        player.m.grabbed_object->update_grabbed(&player, gameScene);

}

void MyGame::pointObject()
{
    DynamicObject* object = player.m.pointed_object;

    if (object)
        return;

    if ((object = selectObject()))
        object->m_selected = true;

    player.m.pointed_object = object;
}

void MyGame::useObject()
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

void MyGame::checkSelectableObject()
{
    pointObject();
    if (player.Input.use)
        useObject();
}

void MyGame::frustumTest()
{
    std::for_each(std::execution::par, gameScene->Meshes.begin(), gameScene->Meshes.end(), [this](Mesh& M){M.onFrustum(player.m.view_frustum);});
}

void MyGame::updateGameLogic()
{
    static float accumulated = 0.0f;
    accumulated += dt;

    while (accumulated >= tick_rate)
    {
        player.reset_rotation();
        processEvents();
        
        if (escape)
            goto skipLoop;
        
        checkSelectableObject();
        player.move(tick_rate);
        player.rotate(tick_rate);
        updatePhysics();
        if (player.should_sound_footstep() && !Mix_Playing(0))
            Mix_PlayChannel(0, footsteps_wave, 0);

skipLoop:
        accumulated -= tick_rate;
        ticks++;
        if (!(ticks%20))
            fps = 1.0/dt;
    }

    player.makeView();
    frustumTest();
}

void MyGame::startFrame()
{
    frame_timer.Start();
    imguiFrameStart();
}

void MyGame::endFrame()
{
    imguiFrameEnd();
    win.swapBuffer();
    frame++;
    runtime += (dt = frame_timer.Count());
}

void MyGame::end_norender()
{
    runtime += (dt = frame_timer.Count());
}

void MyGame::mainLoop()
{
    while(!win.shouldClose())
    {
        startFrame();

        updateGameLogic();

        if(escape)
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

void MyGame::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

int main()
{
    MyGame App(500, 500);
    App.init();
    App.mainLoop();
    App.cleanup();
    return 0;
}