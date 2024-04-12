#include <GL/glew.h>
#include <thread>
#include <algorithm>
#include <execution>
#include "scene.hpp"
#include "scene_helper.hpp"

void scene::combine_as_instance(mesh& A, mesh& Instance)
{
    A.nInstances += Instance.nInstances;
    for (int n = 0; n < Instance.nInstances; n++)
    {
        A.transforms.push_back(Instance.transforms[n]);
    }
    Instance.discard = true;
}

void scene::combine_meshes(mesh& A, mesh& B)
{
    mesh AB;
    A.apply_transform();
    B.apply_transform();
    AB.bounding_box = AABB(A.bounding_box,B.bounding_box);

    AB.vertices.reserve(A.vertices.size() + B.vertices.size());
    AB.vertices.insert(AB.vertices.end(), A.vertices.begin(), A.vertices.end());
    AB.vertices.insert(AB.vertices.end(), B.vertices.begin(), B.vertices.end());
    AB.indices.reserve(A.indices.size() + B.indices.size() + 1);
    AB.indices.insert(AB.indices.end(), A.indices.begin(), A.indices.end());
    AB.indices.push_back(UINT32_MAX);
    for(auto I : B.indices)
        AB.indices.push_back(I+A.vertices.size());

    AB.materialIndex = A.materialIndex;
    AB.Name = A.Name;
    AB.transforms.push_back(glm::mat4(1.0f));
    AB.join_identical_vertices();
    B.discard = true;
    A = AB;
}

mesh scene::process_mesh(const aiMesh* Mesh, const aiMatrix4x4& Transform)
{
    mesh M;
    M.Name = Mesh->mName.C_Str();

    M.vertices.reserve(Mesh->mNumVertices);
    M.indices.reserve(Mesh->mNumFaces*3);

    std::thread Tfaces(process_faces, Mesh, &M.indices);
    std::thread Tverts(process_vertices, Mesh, &M.vertices);

    M.bounding_box.max = cast3D(Mesh->mAABB.mMax);
    M.bounding_box.min = cast3D(Mesh->mAABB.mMin);
    M.transforms.push_back(cast4x4(Transform));
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

void scene::find_instances()
{
    auto it = Meshes.begin();
    auto find_inst = [&it](mesh& M){return *it == M && !M.discard;};
    for(; it < Meshes.end(); it++)
    {
        auto res = it;
        while((res = std::find_if(it+1, Meshes.end(), find_inst)) != Meshes.end())
        {
            scene::combine_as_instance(*it, *res);
        }
    }
}
void scene::find_batchable()
{
    auto it = Meshes.begin();
    auto find_batch = [&it](mesh& M){return M.nInstances == 1 && it->materialIndex == M.materialIndex && !M.discard;};
    for(; it < Meshes.end(); it++)
    {
        if (it->nInstances > 1)
            continue;
        
        auto res = it;
        while((res = std::find_if(it+1, Meshes.end(), find_batch)) != Meshes.end())
        {
            scene::combine_meshes(*it, *res);
        }
    }
}

void scene::load_camera(const aiScene* S)
{
    if(S->HasCameras())
    {
        const auto& aiCam = *S->mCameras[0];
        aiMatrix4x4 Transform = getTransform(aiCam.mName, S);
        aiMatrix3x3 vTransform = aiMatrix3x3(Transform);
        camera::members& cam = m.defaultCamera.m;
        cam.pos = cast3D(Transform * aiCam.mPosition);
        cam.zNear = aiCam.mClipPlaneNear;
        cam.zFar = aiCam.mClipPlaneFar;
        cam.aspect = aiCam.mAspect;
        cam.FOVy = h_fov_to_v_fov(aiCam.mHorizontalFOV, cam.aspect);
        cam.up =    glm::normalize(cast3D(vTransform * aiCam.mUp));
        cam.front = glm::normalize(cast3D(vTransform * aiCam.mLookAt));
        cam.right = glm::normalize(glm::cross(cam.front, cam.up));
    }
    else{
        m.defaultCamera.m.pos = glm::vec3(0.0/0.0);
    }
}

void scene::load_lights(const aiScene* S)
{
    unsigned int nLights = S->mNumLights;
    this->m.init_constants.push_back(nLights);
    this->Lights.resize(nLights);
    for(int n = 0; n < nLights; n++)
    {
        const aiLight *iLight = S->mLights[n];
        aiMatrix4x4 Transform = getTransform(iLight->mName, S);
        this->Lights[n].position = cast3D(Transform * iLight->mPosition);
        this->Lights[n].attenuation.x = iLight->mAttenuationConstant;
        this->Lights[n].attenuation.y = iLight->mAttenuationLinear;
        this->Lights[n].attenuation.z = iLight->mAttenuationQuadratic;
        this->Lights[n].color = cast4D(iLight->mColorDiffuse);
        this->Lights[n].direction = glm::normalize(cast3D(aiMatrix3x3(Transform) * iLight->mDirection));
        this->Lights[n].inner_cone = iLight->mAngleInnerCone;
        this->Lights[n].outer_cone = iLight->mAngleOuterCone;
        this->Lights[n].type = (unsigned int)iLight->mType;
    }
}

inline static 
int get_albedo(const path& P, Material& Mat)
{
    return Mat.loadAlbedo(P).get_slot();
}
inline static 
int get_normal(const path& P, Material& Mat)
{
    return Mat.loadNormal(P).get_slot();
}
inline static 
int get_roughness(const path& P, Material& Mat)
{
    return Mat.loadRoughness(P).get_slot();
}

inline static
int get_emissive(const path& P, Material& Mat)
{
    return Mat.loadEmissive(P).get_slot();
}

void scene::getTexture(aiTextureType Type, const path& parentPath, const aiMaterial* M, material_buffer& matBuff, Material& Mat)
{
    int* texture = nullptr;
    int (*load_fun)(const path&, Material&) = nullptr;

    switch(Type)
    {
        case aiTextureType_BASE_COLOR:
            texture = &matBuff.Albedo;
            load_fun = get_albedo;
            break;
        case aiTextureType_NORMALS:
            texture = &matBuff.Normal;
            load_fun = get_normal;
            break;
        case aiTextureType_DIFFUSE_ROUGHNESS:
            texture = &matBuff.Roughness_metallic;
            load_fun = get_roughness;
            break;
        case aiTextureType_EMISSIVE:
            texture = &matBuff.Emissive;
            load_fun = get_emissive;
        default:
            return;
    }
    if (M->GetTextureCount(Type))
    {
        aiString texture_path;
        M->GetTexture(Type, 0, &texture_path);
        path P = parentPath / texture_path.C_Str();
        *texture = load_fun(P, Mat);
    } else {
        *texture = -1;
    }
}

void scene::load_materials(const aiScene* S, const path& pFile)
{
    unsigned int nMats = S->mNumMaterials;
    path parentPath = pFile.parent_path();
    for(int n = 0; n < nMats; n++)
    {
        const aiMaterial *M = S->mMaterials[n];
        material_buffer matBuff;
        Material Mat;

        getTexture(aiTextureType_BASE_COLOR, parentPath, M, matBuff, Mat);
        getTexture(aiTextureType_NORMALS, parentPath, M, matBuff, Mat);
        getTexture(aiTextureType_DIFFUSE_ROUGHNESS, parentPath, M, matBuff, Mat);
        getTexture(aiTextureType_EMISSIVE, parentPath, M, matBuff, Mat);
        
        m.Materials.push_back(matBuff);
        Materials.push_back(Mat);
    }
}

void scene::clean_meshes()
{
    Meshes.erase(std::remove_if(Meshes.begin(), Meshes.end(), [](mesh& m){return m.discard;}), Meshes.end());
}

void scene::set_max_instances()
{
    for(const auto& M : Meshes)
    {
        m.max_instances = std::max(m.max_instances, M.nInstances);
    }
}

scene* scene::import_scene(const path& pFile) 
{
    Assimp::Importer importer;

    const aiScene* S = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Quality | aiProcess_GenBoundingBoxes);

    if (!S) {
        std::cerr << "Error importing file '" << pFile << "':\n" << importer.GetErrorString();
        return nullptr;
    }

    scene *Scene = new scene;

    Scene->load_lights(S);

    Scene->load_materials(S, pFile);

    Scene->load_camera(S);

    Scene->process_node(S->mRootNode, S);

    Scene->find_instances();

    Scene->find_batchable();

    Scene->clean_meshes();

    Scene->set_max_instances();

    return Scene;
}

void scene::load_DeferredShader(const path& vertex, const path& fragment)
{
    m.deferred_vShader = vertex;
    m.deferred_fShader = fragment;

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
    glClear(GL_COLOR_BUFFER_BIT);
    m.DRenderer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(auto& M : Materials)
        M.bind();

    for(auto& M : Meshes)
    {
        if (!M.nVisibleInstances)
            continue;
        m.mBuffer.bufferSubData(sizeof(material_buffer), 0, &m.Materials[M.materialIndex]);
        Transform_Buffer.bufferSubData(sizeof(glm::mat4)*M.nInstances, transform_offset, M.transforms.data());
        M.render();
    }
    m.PostProcRenderer.m.FB.bind();
    m.sProgram.use();
    m.DRenderer.render(m.PostProcRenderer.m.w, m.PostProcRenderer.m.h);
    m.PostProcRenderer.m.FB.unbind();
    m.PostProcRenderer.m.sProgram.use();
    m.PostProcRenderer.render(m.DRenderer.m.w, m.DRenderer.m.h);
    m.PostProcRenderer.m.sProgram.unuse();
}

void scene::initMeshes()
{
    for(auto& M : Meshes)
        M.init();
}

void scene::initDRenderer(int w, int h)
{
    m.DRenderer.m.vShader.add_specialization_const(m.max_instances, 0);    
    m.DRenderer.init(m.scene_vShader, m.scene_fShader, Materials.size(), w, h);
    m.PostProcRenderer.initPostProc(m.postProcess_vShader, m.postProcess_fShader, w/2,h/2);
}
