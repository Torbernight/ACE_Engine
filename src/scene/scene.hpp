#pragma once

#include <filesystem>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "../renderer/vertexArray.hpp"
#include "../renderer/buffer.hpp"
#include "../renderer/shader.hpp"

#include "../player.hpp"
#include "../renderer/mesh.hpp"
#include "object.hpp"
#include "../renderer/material.hpp"
#include "../renderer/light.hpp"
#include "../renderer/canvas_renderer.hpp"
#include "../renderer/cubemap_renderer.hpp"
#include "../renderer/camera.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

typedef std::filesystem::path path;

#define GL_FLOAT 0x1406
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNIFORM_BUFFER 0x8A11

struct MaterialBuffer
{
    int Albedo;
    int Normal;
    int Roughness_metallic;
    int Emissive;
    int glow = 0;
    float alpha = 1.0f;
};

class Scene
{
    public:
    std::vector<Renderer::Mesh> Meshes;
    std::vector<Renderer::Light> Lights;
    std::vector<Renderer::Material> Materials;
    std::vector<StaticObject>  static_objects;
    std::vector<DynamicObject> dynamic_objects;
    std::vector<Object*> objects;
    std::vector<unsigned int> Mesh_indices;

    struct postprocess
    {
        float Brightness = 0.604;
        float Contrast = 1.828;
        float Saturation = 0.686;
        float Gamma = 2.2;
        float Exposure = 271.7570653294113;
        bool FXAA = true;
    } postprocess; // esto NO debería estar acá

    static Scene* import_scene(const path& pFile);

    void load_DeferredShader(const path& vertex, const path& fragment);
    
    void reload_DeferredShader(Renderer::ShaderStage stage, const std::vector<std::string>& preprocessor_constants);
    void reload_DeferredShader(Renderer::ShaderStage stage);

    void load_SceneShader(const path& vertex, const path& fragment)
    {
        m.scene_vShader = vertex;
        m.scene_fShader = fragment;
    }
    void load_PostProcShader(const path& vertex, const path& fragment)
    {
        m.postProcess_vShader = vertex;
        m.postProcess_fShader = fragment;
    }

    void setUBO();
    void initMeshes();
    void initDRenderer(int w, int h);
    void initDRenderer(int w, int h, int MSAA);

    void initSkybox(const path& vertex, const path& fragment, const path& skybox);

    void resizeViewport(int w, int h)
    {
        m.DRenderer.resize(w,h);
        m.PostProcRenderer.resize(w,h);
    }

    void updatePostProcessBuffer();

    void render(Renderer::Buffer& Transform_Buffer, size_t transform_offset);

    Renderer::CanvasRenderer& get_DRenderer() {
        return m.DRenderer;
    }
    Renderer::CanvasRenderer& get_postProcRenderer() {
        return m.PostProcRenderer;
    }

    const glm::vec3& get_defaultCamPos() const
    {
        return m.cameras[0].get_position();
    }

    const Renderer::Camera& get_defaultCamera() const
    {
        return m.cameras[0];
    }

    unsigned int getMaxInstances() const
    {
        return m.max_instances;
    }

    private:

    struct members
    {
        Renderer::Buffer UBO;
        Renderer::Buffer mBuffer;
        Renderer::Buffer postProcess_buffer;
        Renderer::CanvasRenderer DRenderer;
        Renderer::CanvasRenderer PostProcRenderer;
        Renderer::CubemapRenderer skybox;
        std::vector<unsigned int> init_constants;
        std::vector<MaterialBuffer> Materials;
        Renderer::Shader vShader;
        Renderer::Shader fShader;
        Renderer::shaderProgram sProgram;

        std::vector<Renderer::Camera> cameras;

        path scene_vShader = "";
        path scene_fShader = "";
        path deferred_vShader = "";
        path deferred_fShader = "";
        path postProcess_vShader = "";
        path postProcess_fShader = "";

        unsigned int max_instances = 0;

        members(): 
        UBO(GL_UNIFORM_BUFFER), 
        mBuffer(GL_UNIFORM_BUFFER), 
        postProcess_buffer(GL_UNIFORM_BUFFER), 
        vShader(Renderer::Vertex), 
        fShader(Renderer::Fragment)
        {}
    } m;

    void render_skybox();
    void bind_materials();
    void render_postprocess();
    void render_deferred();
    void render_static_objects(Renderer::Buffer &Transform_Buffer, size_t transform_offset);
    void render_dynamic_objects(Renderer::Buffer &Transform_Buffer, size_t transform_offset);
    static void getTexture(aiTextureType Type, const path& parentPath, const aiMaterial* M, MaterialBuffer& matBuff, Renderer::Material& Mat);
    void set_max_instances();
    void clean_meshes();
    void load_materials(const aiScene* Scene, const path& pFile);
    void load_camera(const aiScene* Scene);
    void load_lights(const aiScene* Scene);
    void find_instances();
    void find_batchable();
    static Renderer::Mesh process_mesh(const aiMesh* Mesh, const aiMatrix4x4& Transform);
    static void combine_meshes(Renderer::Mesh& A, Renderer::Mesh& B);
    static void combine_as_instance(Renderer::Mesh& A, Renderer::Mesh& Instance);
    void process_node(const aiNode* Node, const aiScene* Scene);
    void process_node(const aiNode* Node, const aiScene* Scene, const aiMatrix4x4 &ParentTransform);
};
