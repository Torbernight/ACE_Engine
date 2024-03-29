#include <GL/glew.h>
#include "scene.hpp"

inline static vec3 cast3D(const aiVector3D &V)
{
    return vec3(V.x, V.y, V.z);
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
    aiMatrix4x4 Transform = Node->mTransformation;
    for (int n = 0; n < Node->mNumMeshes; n++)
    {
        unsigned int index = Node->mMeshes[n];
        Meshes.push_back(process_mesh(Scene->mMeshes[index], Transform));
    }

    for (int n = 0; n < Node->mNumChildren; n++)
        process_node(Node->mChildren[n], Scene, Node->mTransformation);
}

void scene::process_node(const aiNode* Node, const aiScene* Scene, const aiMatrix4x4 &ParentTransform)
{
    aiMatrix4x4 Transform = ParentTransform * Node->mTransformation;

    for (int n = 0; n < Node->mNumMeshes; n++)
    {
        unsigned int index = Node->mMeshes[n];
        Meshes.push_back(process_mesh(Scene->mMeshes[index], Transform));
    }

    for (int n = 0; n < Node->mNumChildren; n++)
        process_node(Node->mChildren[n], Scene, Node->mTransformation);
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

scene* scene::import_scene(const path& pFile) 
{
    Assimp::Importer importer;

    const aiScene* S = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Fast);

    if (!S) {
        std::cerr << "Error importing file '" << pFile << "':\n" << importer.GetErrorString();
        return nullptr;
    }

    scene *Scene = new scene;

    unsigned int nLights = S->mNumLights;
    unsigned int nMats = S->mNumMaterials;

    Scene->m.init_constants.push_back(nLights);

    Scene->Lights.resize(nLights);

    for(int n = 0; n < nLights; n++)
    {
        const aiLight *iLight = S->mLights[n];
        aiMatrix4x4 Transform = getTransform(iLight->mName, S);
        Scene->Lights[n].position = cast3D(Transform * iLight->mPosition);
        Scene->Lights[n].attenuation.x = iLight->mAttenuationConstant;
        Scene->Lights[n].attenuation.y = iLight->mAttenuationLinear;
        Scene->Lights[n].attenuation.z = iLight->mAttenuationQuadratic;
        Scene->Lights[n].color = cast4D(iLight->mColorDiffuse);
        Scene->Lights[n].direction = glm::normalize(cast3D(aiMatrix3x3(Transform) * iLight->mDirection));
        Scene->Lights[n].inner_cone = iLight->mAngleInnerCone;
        Scene->Lights[n].outer_cone = iLight->mAngleOuterCone;
        Scene->Lights[n].type = (unsigned int)iLight->mType;
    }
    for(int n = 0; n < nMats; n++)
    {
        aiMaterial *M = S->mMaterials[n];
        
        if (0 == S->mMaterials[n]->GetTextureCount(aiTextureType_DIFFUSE))
            continue;

        aiString TexturePath;
        M->GetTexture(aiTextureType_DIFFUSE, 0, &TexturePath);
        if (TexturePath.length == 0)
         continue;
        Material Mat;
        Mat.load(pFile.parent_path() / TexturePath.C_Str());
        Scene->Materials.push_back(Mat);
    }

    if(S->HasCameras())
    {
        aiMatrix4x4 Transform = getTransform(S->mCameras[0]->mName, S);
        Scene->m.defaultCamPos = cast3D(Transform * S->mCameras[0]->mPosition);
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

void scene::updatePostProcessBuffer()
{
    m.postProcess_buffer.bufferData(sizeof(postprocess), &postprocess, GL_DYNAMIC_DRAW);
}

void scene::setUBO()
{
    m.UBO.generate();
    m.UBO.bufferData(Lights.size()*sizeof(light), Lights.data(), GL_DYNAMIC_DRAW);
    m.UBO.bindBufferBase(2);

    for(auto &M : Materials)
    {
        material_buffer mat;
        mat.Albedo = M.get_albedo_unit();
        m.Materials.push_back(mat);
    }

    m.mBuffer.generate();
    m.mBuffer.bufferData(m.Materials.size()*sizeof(material_buffer)+4UL, nullptr, GL_DYNAMIC_DRAW);
    m.mBuffer.bindBufferBase(3);
    m.mBuffer.bufferSubData(m.Materials.size()*sizeof(material_buffer), 4UL, m.Materials.data());

    m.postProcess_buffer.generate();
    m.postProcess_buffer.bufferData(sizeof(postprocess), &postprocess, GL_DYNAMIC_DRAW);
    m.postProcess_buffer.bindBufferBase(4);
}

void scene::render(buffer &Transform_Buffer, size_t transform_offset)
{
    m.DRenderer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(auto& M : Materials)
    {
        M.bind();
    }
    for(auto& M : Meshes)
    {
        m.mBuffer.bufferSubData(4UL, 0, &M.materialIndex);
        Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &M.transform[0][0]);
        M.render();
    }
    m.DRenderer.unbind();
    m.PostProcRenderer.m.FB.bind();
    m.sProgram.use();
    m.DRenderer.render();
    m.sProgram.unuse();
    m.PostProcRenderer.m.FB.unbind();
    m.PostProcRenderer.m.sProgram.use();
    m.PostProcRenderer.render();
    m.PostProcRenderer.m.sProgram.unuse();

}

void scene::initMeshes()
{
    for(auto& M : Meshes)
    {
        M.init();
    }
}

void scene::initDRenderer(int w, int h)
{
    m.DRenderer.init("shader/scene.vert.spv", "shader/scene.frag.spv", Materials.size(), w, h);
    m.PostProcRenderer.initPostProc("shader/deferred.vert.spv", "shader/postprocess.frag.spv", w,h);
}
