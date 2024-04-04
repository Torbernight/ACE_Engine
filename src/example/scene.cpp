#include <GL/glew.h>
#include <thread>
#include <algorithm>
#include "scene.hpp"
#include "scene_helper.hpp"


mesh scene::combine_meshes(mesh& A, mesh& B)
{
    mesh AB;
    A.apply_transform();
    B.apply_transform();

    AB.vertices.reserve(A.vertices.size() + B.vertices.size());
    AB.vertices.insert(AB.vertices.end(), A.vertices.begin(), A.vertices.end());
    AB.vertices.insert(AB.vertices.end(), B.vertices.begin(), B.vertices.end());
    AB.indices.reserve(A.indices.size() + B.indices.size() + 1);
    AB.indices.insert(AB.indices.end(), A.indices.begin(), A.indices.end());
    AB.indices.push_back(UINT32_MAX);
    for(auto I : B.indices)
        AB.indices.push_back(I+A.vertices.size());

    AB.materialIndex = A.materialIndex;
    AB.transform = glm::mat4(1.0f);
    AB.join_identical_vertices();
    return AB;
}

mesh scene::process_mesh(const aiMesh* Mesh, const aiMatrix4x4& Transform)
{
    mesh M;

    M.vertices.reserve(Mesh->mNumVertices);
    M.indices.reserve(Mesh->mNumFaces*3);
    std::thread Tfaces(process_faces, Mesh, &M.indices);
    std::thread Tverts(process_vertices, Mesh, &M.vertices);

    M.bounding_box.max = cast3D(Mesh->mAABB.mMax);
    M.bounding_box.min = cast3D(Mesh->mAABB.mMin);
    M.transform = cast4x4(Transform);
    M.materialIndex = Mesh->mMaterialIndex;

    Tfaces.join();
    Tverts.join();
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
        material_buffer matBuff;
        

        matBuff.Albedo = M->GetTextureCount(aiTextureType_BASE_COLOR) == 0 ? -1 : 0;
        matBuff.Normal = M->GetTextureCount(aiTextureType_NORMALS) == 0 ? -1 : 0;
        matBuff.Roughness_metallic = M->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) == 0 ? -1 : 0;

        Material Mat;

        if (0 == matBuff.Albedo){
            aiString AlbedoPath;
            M->GetTexture(aiTextureType_BASE_COLOR, 0, &AlbedoPath);
            matBuff.Albedo = Mat.loadAlbedo(pFile.parent_path() / AlbedoPath.C_Str()).get_slot();
        }
        if (0 == matBuff.Normal){
            aiString NormalPath;
            M->GetTexture(aiTextureType_NORMALS, 0, &NormalPath);
            matBuff.Normal = Mat.loadNormal(pFile.parent_path() / NormalPath.C_Str()).get_slot();
        }
        if (0 == matBuff.Roughness_metallic){
            aiString RoughnessPath;
            M->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &RoughnessPath);
            matBuff.Roughness_metallic = Mat.loadRoughness(pFile.parent_path() / RoughnessPath.C_Str()).get_slot();
        }

        Scene->m.Materials.push_back(matBuff);
        Scene->Materials.push_back(Mat);
    }

    if(S->HasCameras())
    {
        aiMatrix4x4 Transform = getTransform(S->mCameras[0]->mName, S);
        Scene->m.defaultCamPos = cast3D(Transform * S->mCameras[0]->mPosition);
    }

    Scene->process_node(S->mRootNode, S);

    for(auto it = Scene->Meshes.begin(); it < Scene->Meshes.end(); it++)
    {
        auto res = it;

        while(res != Scene->Meshes.end())
        {
            res = std::find_if(it+1, Scene->Meshes.end(), [it](mesh& M){return it->materialIndex == M.materialIndex && !M.discard;});
            if (res == Scene->Meshes.end())
                break;

            *it = scene::combine_meshes(*it, *res);
            res->discard = true;
        }
    }

    Scene->Meshes.erase(std::remove_if(Scene->Meshes.begin(), Scene->Meshes.end(), [](mesh& m){return m.discard;}));

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

    m.mBuffer.generate();
    m.mBuffer.bufferData(sizeof(material_buffer), nullptr, GL_DYNAMIC_DRAW);
    m.mBuffer.bindBufferBase(3);

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
        m.mBuffer.bufferSubData(sizeof(material_buffer), 0, &m.Materials[M.materialIndex]);
        Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &M.transform[0][0]);
        M.render();
    }
    m.DRenderer.unbind();
    m.PostProcRenderer.m.FB.bind();
    m.sProgram.use();
    m.DRenderer.render(m.PostProcRenderer.m.w, m.PostProcRenderer.m.h);
    m.sProgram.unuse();
    m.PostProcRenderer.m.FB.unbind();
    m.PostProcRenderer.m.sProgram.use();
    m.PostProcRenderer.render(m.DRenderer.m.w, m.DRenderer.m.h);
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
    m.PostProcRenderer.initPostProc("shader/deferred.vert.spv", "shader/postprocess.frag.spv", w/2,h/2);
}
