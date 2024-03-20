#pragma once

#include <iostream>
#include <filesystem>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "../vertexArray.hpp"
#include "../buffer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

typedef std::filesystem::path path;

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;

#define GL_FLOAT 0x1406
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

class mesh
{
    public:
    struct vertex
    {
        vec3 position;
        vec2 texture;
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
        {1, 2 , GL_FLOAT, 0, sizeof(vertex), offsetof(vertex, texture)},
        {2, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, normal)},
        {3, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, tangent)},
        {4, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, bitangent)}
        }), 
    
    VBO(GL_ARRAY_BUFFER), EBO(GL_ELEMENT_ARRAY_BUFFER)
    {}

    std::vector<vertex> vertices;
    glm::mat4 transform;
    unsigned int materialIndex = 0;

    void init();
    void render();

};

inline static vec3 cast3D(const aiVector3D &V)
{
    return {V.x, V.y, V.z};
}
inline static vec2 cast2D(const aiVector3D &V)
{
    return {V.x, V.y};
}

mesh process_mesh(const aiMesh& Mesh)
{
    mesh M;
    M.vertices.reserve(Mesh.mNumVertices);
    for (int n = 0; n < Mesh.mNumVertices; n++)
    {
        mesh::vertex V;
        V.position  = cast3D(Mesh.mVertices[n]);
        V.texture   = cast2D(Mesh.mTextureCoords[0][n]);
        V.normal    = cast3D(Mesh.mNormals[n]);
        V.tangent   = cast3D(Mesh.mTangents[n]);
        V.bitangent = cast3D(Mesh.mBitangents[n]);
    }
    M.materialIndex = Mesh.mMaterialIndex;
}



bool import_model(const path& pFile) 
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Fast);

    if (nullptr == scene) 
    {
        std::cerr << "Error importing file '" << pFile << "':\n" << importer.GetErrorString();
        return false;
    }


    //scene->mRootNode->mNumMeshes;
    //scene->mMeshes[0]->mNumVertices
    
    return true;
}