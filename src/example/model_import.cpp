#include <GL/glew.h>
#include "model_import.hpp"

inline static vec3 cast3D(const aiVector3D &V)
{
    return vec3(V.x, V.y, V.z);
}
inline static vec3 cast3D(const aiColor3D &V)
{
    return vec3(V.r, V.g, V.b);
}
inline static vec4 cast4D(const aiColor3D &V)
{
    return vec4(V.r, V.g, V.b, 1.0f);
}
inline static vec2 cast2D(const aiVector3D &V)
{
    return vec2(V.x, V.y);
}
inline static glm::mat4 cast4x4(const aiMatrix4x4 &M)
{
    glm::mat4 Result(1);
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            Result[i][j] = M[j][i];
        }
    }
    return Result;
}

void mesh::init()
{
    VBO.generate();
    VBO.bufferData(vertices.size()*sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
    VAO.generateVAO();
    VAO.setVertexAttributes(VBO, 0, sizeof(vertex));
    VAO.bind();
    EBO.generate();
    EBO.bind();
    EBO.bufferData(indices.size()*4UL, indices.data(), GL_STATIC_DRAW);
    VAO.unbind();
}

void mesh::render()
{
    VAO.bind();
    EBO.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    VAO.unbind();
}

mesh scene::process_mesh(const aiMesh* Mesh, const aiMatrix4x4& Transform)
{
    mesh M;
    M.transform = cast4x4(Transform);
    M.vertices.reserve(Mesh->mNumVertices);
    
    for (int n = 0; n < Mesh->mNumFaces; n++)
    {
        const aiFace &F = Mesh->mFaces[n];
        for(int i = 0; i < F.mNumIndices; i++)
        {
            M.indices.push_back(F.mIndices[i]);
        }
    }

    for (int n = 0; n < Mesh->mNumVertices; n++)
    {
        mesh::vertex V;
        V.position  = cast3D(Mesh->mVertices[n]);
        V.texcoord  = cast2D(Mesh->mTextureCoords[0][n]);
        V.normal    = cast3D(Mesh->mNormals[n]);
        V.tangent   = cast3D(Mesh->mTangents[n]);
        V.bitangent = cast3D(Mesh->mBitangents[n]);
        M.vertices.push_back(V);
    }
    M.materialIndex = Mesh->mMaterialIndex;
    return M;
}

void scene::process_node(const aiNode* Node, const aiScene* Scene)
{
    for (int n = 0; n < Node->mNumMeshes; n++)
    {
        unsigned int index = Node->mMeshes[n];
        aiMatrix4x4 Transform = Node->mTransformation;
        Meshes.push_back(process_mesh(Scene->mMeshes[index], Transform));
    }

    for (int n = 0; n < Node->mNumChildren; n++)
        process_node(Node->mChildren[n], Scene, Node->mTransformation);
}

void scene::process_node(const aiNode* Node, const aiScene* Scene, const aiMatrix4x4 &ParentTransform)
{
    for (int n = 0; n < Node->mNumMeshes; n++)
    {
        unsigned int index = Node->mMeshes[n];
        aiMatrix4x4 Transform = ParentTransform * Node->mTransformation;
        Meshes.push_back(process_mesh(Scene->mMeshes[index], Transform));
    }

    for (int n = 0; n < Node->mNumChildren; n++)
        process_node(Node->mChildren[n], Scene, Node->mTransformation);
}


scene* scene::import_scene(const path& pFile) 
{
    Assimp::Importer importer;

    const aiScene* S = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Fast);

    if (!S) 
    {
        std::cerr << "Error importing file '" << pFile << "':\n" << importer.GetErrorString();
        return nullptr;
    }

    scene *Scene = new scene;

    unsigned int nLights = S->mNumLights;
    unsigned int nMats = S->mNumMaterials;

    Scene->m.init_constants.push_back(nLights);
    Scene->m.init_constants.push_back(nMats);

    Scene->Lights.resize(nLights);

    for(int n = 0; n < nLights; n++)
    {
        const aiLight *iLight = S->mLights[n];
        Scene->Lights[n].position = cast3D(iLight->mPosition);
        Scene->Lights[n].attenuation.x = iLight->mAttenuationConstant;
        Scene->Lights[n].attenuation.y = iLight->mAttenuationLinear;
        Scene->Lights[n].attenuation.z = iLight->mAttenuationQuadratic;
        Scene->Lights[n].color = cast4D(iLight->mColorDiffuse);
        //Scene->Lights[n].color_ambient = cast3D(iLight->mColorAmbient);
        //Scene->Lights[n].color_specular = cast3D(iLight->mColorSpecular);
        Scene->Lights[n].direction = cast3D(iLight->mDirection);
        Scene->Lights[n].inner_cone = iLight->mAngleInnerCone;
        Scene->Lights[n].outer_cone = iLight->mAngleOuterCone;
        Scene->Lights[n].type = (unsigned int)iLight->mType;
    }
    for(int n = 0; n < nMats; n++)
    {
        aiMaterial *M = S->mMaterials[n];
        aiString TexturePath;
        M->GetTexture(aiTextureType_DIFFUSE, 0, &TexturePath);
        if (TexturePath.length == 0)
         continue;
        Material Mat;
        Mat.load(pFile.parent_path() / TexturePath.C_Str());
        Scene->Materials.push_back(Mat);
    }

    Scene->process_node(S->mRootNode, S);
    
    return Scene;
}

void scene::loadShader(const path& vertex, const path& fragment)
{
    m.vShader.loadBinary(vertex);
    m.fShader.loadBinary(fragment);
    for(int i = 0; i < m.init_constants.size(); i++)
    {
        m.fShader.add_specialization_const(m.init_constants[i], i);
    }
    m.vShader.specialize();
    m.fShader.specialize();
    m.sProgram.attachShader(m.vShader);
    m.sProgram.attachShader(m.fShader);
    m.sProgram.linkProgram();
}

void scene::setUBO()
{
    m.UBO.generate();
    m.UBO.bufferData(Lights.size()*sizeof(light), Lights.data(), GL_DYNAMIC_DRAW);
    m.UBO.bindBufferBase(2);
}

void scene::render(buffer &Transform_Buffer, size_t transform_offset)
{
    m.sProgram.use();
    for(auto& M : Meshes)
    {
        Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &M.transform[0][0]);
        Materials[M.materialIndex].bind();
        M.render();
    }
    m.sProgram.unuse();
}

void scene::initMeshes()
{
    for(auto& M : Meshes)
    {
        M.init();
    }
}