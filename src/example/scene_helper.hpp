#pragma once

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
void process_vertices(const aiMesh* Mesh, std::vector<mesh::vertex> *vertices)
{
    for (int n = 0; n < Mesh->mNumVertices; n++)
    {
        mesh::vertex V;
        V.position  = cast3D(Mesh->mVertices[n]);
        V.texcoord  = cast2D(Mesh->mTextureCoords[0][n]);
        V.normal    = cast3D(Mesh->mNormals[n]);
        V.tangent   = cast3D(Mesh->mTangents[n]);
        V.bitangent = cast3D(Mesh->mBitangents[n]);
        vertices->push_back(V);
    }
}

inline static
float h_fov_to_v_fov(float hfov, float aspect)
{
    return 2 * atanf(tanf(hfov*0.5f)*aspect);
}