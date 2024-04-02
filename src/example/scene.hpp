#pragma once

#include <iostream>
#include <filesystem>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "../shader.hpp"

#include "mesh.hpp"
#include "material.hpp"
#include "light.hpp"
#include "deferred_renderer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

typedef std::filesystem::path path;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

#define GL_FLOAT 0x1406
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNIFORM_BUFFER 0x8A11

struct material_buffer
{
    int Albedo;
    int Normal;
    int Roughness;
};

class scene
{
    public:
    std::vector<mesh> Meshes;
    std::vector<light> Lights;
    std::vector<Material> Materials;
    

    struct postprocess{
        float Brightness = 0.604;
        float Contrast = 1.828;
        float Saturation = 0.686;
        float Gamma = 2.302;
    } postprocess;

    static scene* import_scene(const path& pFile);
    void loadShader(const path& vertex, const path& fragment);
    void setUBO();
    void initMeshes();
    void initDRenderer(int w, int h);

    void resizeViewport(int w, int h)
    {
        m.DRenderer.resize(w,h);
        m.PostProcRenderer.resize(w/2,h/2);
    }

    void updatePostProcessBuffer();

    void render(buffer& Transform_Buffer, size_t transform_offset);

    deferred_renderer& get_DRenderer() {
        return m.DRenderer;
    }
    deferred_renderer& get_postProcRenderer() {
        return m.PostProcRenderer;
    }

    const glm::vec3& get_defaultCamPos() const
    {
        return m.defaultCamPos;
    }

    private:

    struct members
    {
        buffer UBO;
        buffer mBuffer;
        buffer postProcess_buffer;
        deferred_renderer DRenderer;
        deferred_renderer PostProcRenderer;
        std::vector<unsigned int> init_constants;
        std::vector<material_buffer> Materials;
        Shader vShader;
        Shader fShader;
        shaderProgram sProgram;

        glm::vec3 defaultCamPos = glm::vec3(0.0f/0.0f);

        members(): 
        UBO(GL_UNIFORM_BUFFER), 
        mBuffer(GL_UNIFORM_BUFFER), 
        postProcess_buffer(GL_UNIFORM_BUFFER), 
        vShader(Vertex), 
        fShader(Fragment)
        {}
    }m;

    static mesh process_mesh(const aiMesh* Mesh, const aiMatrix4x4& Transform);
    void process_node(const aiNode* Node, const aiScene* Scene);
    void process_node(const aiNode* Node, const aiScene* Scene, const aiMatrix4x4 &ParentTransform);
};
