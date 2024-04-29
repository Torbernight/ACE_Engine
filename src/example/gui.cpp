#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "example.hpp"

#define XYZW(V) V.x, V.y, V.z, V.w
#define XYZ(V) V.x, V.y, V.z
#define XY(V) V.x, V.y


void MyGame::imgui_frame_start()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void MyGame::imgui_frame_end()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MyGame::info_window()
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


void MyGame::show_config()
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
