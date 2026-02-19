#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "example.hpp"

void MyGame::processEvents()
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
                player.Input.hasClicked = player.Input.lclick != click && click;
                player.Input.lclick = click;
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

void MyGame::processMouseWheel(const SDL_MouseWheelEvent& E)
{
    float FOV = glm::radians(glm::degrees(player.getFOV())+E.y);
    player.setFOV(glm::clamp(FOV,0.0f, M_PIf));
    TransformBuffer.bufferSubData(sizeof(glm::mat4), 0, &player.getProj()[0][0]);
}

void MyGame::processWindowEvents(const SDL_WindowEvent &E)
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

void MyGame::processMouseEvents(const SDL_MouseMotionEvent& E)
{
    glm::vec2 r(E.xrel, -E.yrel);
    player.m.delta_rotation = r*mouse_sensitivity*0.01f;
}

void MyGame::processKeyEvents(const SDL_KeyboardEvent &E)
{
    bool pressed = E.type == SDL_KEYDOWN;
    switch(E.keysym.sym)
    {
        case SDLK_w:
        case SDLK_UP:
            player.Input.forward = pressed;
            break;

        case SDLK_s:
        case SDLK_DOWN:
            player.Input.backward = pressed;
            break;

        case SDLK_d:
        case SDLK_RIGHT:
            player.Input.right = pressed;
            break;

        case SDLK_a:
        case SDLK_LEFT:
            player.Input.left = pressed;
            break;

        case SDLK_SPACE:
            player.Input.up = pressed;
            break;

        case SDLK_LCTRL:
            player.Input.down = pressed;
            break;
        
        case SDLK_LSHIFT:
            player.Input.sprint = pressed;
            break;

        case SDLK_e:
            player.Input.use = !player.Input.use_key && pressed;
            player.Input.use_key = pressed;
            break;

        case SDLK_ESCAPE:
            if (pressed)
                escape = !escape;
            break;
    }
}
