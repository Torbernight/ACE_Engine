#pragma once

#include <glad/gl.h>
#include "buffer.hpp"
#include "camera.hpp"
#include "cubemap_renderer.hpp"
#include "canvas_renderer.hpp"
#include "light.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"

namespace Renderer 
{
    typedef int TextureID;

    typedef size_t CameraID;
    typedef size_t MaterialID;
    typedef size_t LightID;

    struct MaterialParameter
    {
        TextureID Albedo;
        TextureID Normal;
        TextureID Roughness_metallic;
        TextureID Emissive;
        int glow = 0;
        float alpha = 1.0f;
    };

   
    class State 
    {
        public:

        State& get()
        {
            static State s = State();
            return s;
        }

        LightID add_light(const Light& light);
        Light& get_light(LightID ID);
        const Light& get_light(LightID ID) const;

        CameraID add_camera(const Camera& camera);
        Camera& get_camera(CameraID ID);
        const Camera& get_camera(CameraID ID) const;

        MaterialID add_material(const Material& material);
        Material& get_material(MaterialID ID);
        const Material& get_material(MaterialID ID) const;

        
        private:

        State() : 
            LightsBuffer(GL_UNIFORM_BUFFER),
            MaterialBuffer(GL_UNIFORM_BUFFER),
            PostProcessBuffer(GL_UNIFORM_BUFFER),
            VertexShader(ShaderStage_Vertex),
            FragmentShader(ShaderStage_Fragment)
            {}

        Renderer::Buffer LightsBuffer;
        Renderer::Buffer MaterialBuffer;
        Renderer::Buffer PostProcessBuffer;
        
        Renderer::CanvasRenderer DeferredPipeline;
        Renderer::CanvasRenderer PostProcessPipeline;
        Renderer::CubemapRenderer SkyboxPipeline;
        
        std::vector<unsigned int> initConstants;
        std::vector<MaterialParameter> MaterialParameters;
        
        Renderer::Shader VertexShader;
        Renderer::Shader FragmentShader;
        Renderer::shaderProgram ShaderProgram;

        std::vector<Renderer::Camera> Cameras;
        std::vector<Renderer::Light> Lights;
        std::vector<Renderer::Material> Materials;



    };

}