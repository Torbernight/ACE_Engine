#pragma once

#include <iostream>
#include <filesystem>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "../shader.hpp"

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
    unsigned int Albedo;
    unsigned int Normal;
    unsigned int AO;
    unsigned int junk;
};

class mesh
{
    public:
    struct vertex
    {
        vec3 position;
        vec2 texcoord;
        vec3 normal;
        vec3 tangent;
        vec3 bitangent;
    };
    
    vertexArray VAO;
    buffer VBO, EBO;

    mesh()
    : 
    VAO({
        {0, 3 , GL_FLOAT, 0, sizeof(vertex), 0},
        {1, 2 , GL_FLOAT, 0, sizeof(vertex), offsetof(vertex, texcoord)},
        {2, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, normal)},
        {3, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, tangent)},
        {4, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, bitangent)}
        }), 
    
    VBO(GL_ARRAY_BUFFER), EBO(GL_ELEMENT_ARRAY_BUFFER)
    {}

    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    glm::mat4 transform;
    unsigned int materialIndex = 0;

    void init();
    void render();

};

class scene
{
    public:
    std::vector<mesh> Meshes;
    std::vector<light> Lights;
    std::vector<Material> Materials;

    static scene* import_scene(const path& pFile);
    void loadShader(const path& vertex, const path& fragment);
    void setUBO();
    void initMeshes();
    void initDRenderer(int w, int h);

    void render(buffer& Transform_Buffer, size_t transform_offset);

    deferred_renderer& get_DRenderer() {
        return m.DRenderer;
    }

    private:

    struct members
    {
        buffer UBO;
        buffer mBuffer;
        deferred_renderer DRenderer;
        std::vector<unsigned int> init_constants;
        std::vector<material_buffer> Materials;
        Shader vShader;
        Shader fShader;
        shaderProgram sProgram;

        members(): UBO(GL_UNIFORM_BUFFER), mBuffer(GL_UNIFORM_BUFFER), vShader(Vertex), fShader(Fragment) {}
    }m;

    static mesh process_mesh(const aiMesh* Mesh, const aiMatrix4x4& Transform);
    void process_node(const aiNode* Node, const aiScene* Scene);
    void process_node(const aiNode* Node, const aiScene* Scene, const aiMatrix4x4 &ParentTransform);
};
