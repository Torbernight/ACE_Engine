#pragma once

#include "scene.hpp"

inline static
glm::vec3 cast3D(const aiVector3D &V)
{
    return glm::vec3(V.x, V.y, V.z);
}
inline static
glm::vec3 cast3D(const aiColor3D &V)
{
    return glm::vec3(V.r, V.g, V.b);
}
inline static
glm::vec4 cast4D(const aiColor3D &V)
{
    return glm::vec4(V.r, V.g, V.b, 1.0f);
}
inline static
glm::vec2 cast2D(const aiVector3D &V)
{
    return glm::vec2(V.x, V.y);
}
inline static
glm::mat4 cast4x4(const aiMatrix4x4 &M)
{
    glm::mat4 Result(1);
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            Result[i][j] = M[j][i];
        }
    }
    return Result;
}

inline static
void process_faces(const aiMesh* Mesh, std::vector<uint32_t> *indices)
{
    for (int n = 0; n < Mesh->mNumFaces; n++)
    {
        const aiFace &F = Mesh->mFaces[n];
        for(int i = 0; i < F.mNumIndices; i++)
        {
            indices->push_back(F.mIndices[i]);
        }
    }
}

inline static
void process_vertices(const aiMesh* mesh, std::vector<Renderer::Mesh::Vertex> *vertices)
{
    for (int n = 0; n < mesh->mNumVertices; n++)
    {
        Renderer::Mesh::Vertex V;
        V.position  = cast3D(mesh->mVertices[n]);
        V.texcoord  = cast2D(mesh->mTextureCoords[0][n]);
        V.normal    = cast3D(mesh->mNormals[n]);
        V.tangent   = cast3D(mesh->mTangents[n]);
        V.bitangent = cast3D(mesh->mBitangents[n]);
        vertices->push_back(V);
    }
}

inline static
float h_fov_to_v_fov(float hfov, float aspect)
{
    return 2 * atanf(tanf(hfov*0.5f)*aspect);
}


inline static
aiMatrix4x4 getTransform(const aiString& nodeName, const aiScene* Scene)
{
    aiMatrix4x4 Transform = aiMatrix4x4({1.0,1.0,1.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0});
    aiNode* Node = Scene->mRootNode->FindNode(nodeName);

    while(nullptr != Node) {
        Transform = Transform * Node->mTransformation;
        Node = Node->mParent;
    }

    return Transform;
}