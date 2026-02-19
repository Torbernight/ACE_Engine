#include <GL/glew.h>
#include <thread>
#include <algorithm>
#include <execution>
#include "scene.hpp"
#include "misc/scene_aux.hpp"
#include "shader_compiler.hpp"

void Scene::combine_as_instance(Mesh& A, Mesh& Instance)
{
    //A.m.nInstances += Instance.m.nInstances;
    //for (int n = 0; n < Instance.m.nInstances; n++)
    //{
    //    A.m.transforms.push_back(Instance.m.transforms[n]);
    //}
    //Instance.m.discard = true;
}

void Scene::combine_meshes(Mesh& A, Mesh& B)
{
    Mesh AB;
    A.apply_transform();
    B.apply_transform();
    AB.m.bounding_box = AABB(A.m.bounding_box,B.m.bounding_box);

    AB.m.vertices.reserve(A.m.vertices.size() + B.m.vertices.size());
    AB.m.vertices.insert(AB.m.vertices.end(), A.m.vertices.begin(), A.m.vertices.end());
    AB.m.vertices.insert(AB.m.vertices.end(), B.m.vertices.begin(), B.m.vertices.end());
    AB.m.indices.reserve(A.m.indices.size() + B.m.indices.size() + 1);
    AB.m.indices.insert(AB.m.indices.end(), A.m.indices.begin(), A.m.indices.end());
    AB.m.indices.push_back(UINT32_MAX);
    for(auto I : B.m.indices)
        AB.m.indices.push_back(I+A.m.vertices.size());

    AB.m.materialIndex = A.m.materialIndex;
    AB.m.name = A.m.name;
    AB.m.transform = glm::mat4(1.0f);
    AB.join_identical_vertices();
    B.m.discard = true;
    A = AB;
}

Mesh Scene::process_mesh(const aiMesh* mesh, const aiMatrix4x4& transform)
{
    Mesh M;
    M.m.name = mesh->mName.C_Str();

    M.m.vertices.reserve(mesh->mNumVertices);
    M.m.indices.reserve(mesh->mNumFaces*3);

    std::thread Tfaces(process_faces, mesh, &M.m.indices);
    std::thread Tverts(process_vertices, mesh, &M.m.vertices);

    M.m.bounding_box.max = cast3D(mesh->mAABB.mMax);
    M.m.bounding_box.min = cast3D(mesh->mAABB.mMin);
    M.m.transform = cast4x4(transform);
    M.m.materialIndex = mesh->mMaterialIndex;

    Tfaces.join();
    Tverts.join();
    return M;
}

#define XYZ(V) V.x << " " << V.y << " " << V.z

void process_metadata(const aiMetadata *Metadata, int level = 0)
{
    for (int n = 0; n < Metadata->mNumProperties; n++){
        void* data = Metadata->mValues[n].mData;
        for(int m = 0; m < level+1; m++){
            std::cout << "\t";
        }
        switch(Metadata->mValues[n].mType)
        {
            case AI_BOOL:
                std::cout << "type BOOL: " << Metadata->mKeys[n].C_Str() << ": " << ((*(bool*)data) ? "true" : "false") << "\n";
                break;
            case AI_INT32:
                std::cout << "type INT32: " << Metadata->mKeys[n].C_Str() << ": " << *(int32_t*)data << "\n";
                break;
            case AI_UINT64:
                std::cout << "type UINT64: " << Metadata->mKeys[n].C_Str() << ": " << *(uint64_t*)data << "\n";
                break;
            case AI_FLOAT:
                std::cout << "type FLOAT: " << Metadata->mKeys[n].C_Str() << ": " << *(float*)data << "\n";
                break;
            case AI_DOUBLE:
                std::cout << "type DOUBLE: " << Metadata->mKeys[n].C_Str() << ": " << *(double*)data << "\n";
                break;
            case AI_AISTRING:
                std::cout << "type AISTRING: " << Metadata->mKeys[n].C_Str() << ": " << ((aiString*)data)->C_Str() << "\n";
                break;
            case AI_AIVECTOR3D:
                std::cout << "type AIVECTOR3D: " << Metadata->mKeys[n].C_Str() << ": " << XYZ((*(aiVector3D*)data)) << "\n";
                break;
            case AI_AIMETADATA:
                std::cout << "\ttype AIMETADATA:\n";
                process_metadata((aiMetadata*)data, level+1);
                break;
            case AI_INT64:
                std::cout << "\ttype INT64: " << Metadata->mKeys[n].C_Str() << ": " << *(int64_t*)data << "\n";
                break;
            case AI_UINT32:
                std::cout << "\ttype UINT32: " << Metadata->mKeys[n].C_Str() << ": " << *(uint32_t*)data << "\n";
                break;
            default:
                std::cout << "\ttype UNKNOWN\n";
            break;
        }
    }
}

void Scene::process_node(const aiNode* Node, const aiScene* Scene)
{
    bool grabbable = false;
    bool mesh_static = true;
    if (Node->mMetaData){
        Node->mMetaData->Get<bool>("grabbable", grabbable);
        Node->mMetaData->Get<bool>("static", mesh_static);
    }
    aiMatrix4x4 Transform = Node->mTransformation;

    for (int n = 0; n < Node->mNumMeshes; n++)
    {
        unsigned int index = Node->mMeshes[n];
        Meshes.push_back(process_mesh(Scene->mMeshes[index], Transform));
        if (mesh_static){
            static_objects.push_back(StaticObject(index));
        } else if (grabbable) {
            dynamic_objects.push_back(DynamicObject(index));
        }
    }

    for (int n = 0; n < Node->mNumChildren; n++)
        process_node(Node->mChildren[n], Scene, Node->mTransformation);
}
#undef XYZ

void Scene::process_node(const aiNode* Node, const aiScene* Scene, const aiMatrix4x4 &ParentTransform)
{
    bool grabbable = false;
    bool mesh_static = true;
    if (Node->mMetaData){
        Node->mMetaData->Get<bool>("grabbable", grabbable);
        Node->mMetaData->Get<bool>("static", mesh_static);
    }
    aiMatrix4x4 Transform = ParentTransform * Node->mTransformation;

    for (int n = 0; n < Node->mNumMeshes; n++)
    {
        unsigned int index = Node->mMeshes[n];
        Meshes.push_back(process_mesh(Scene->mMeshes[index], Transform));
        if (mesh_static){
            static_objects.push_back(StaticObject(index));
        } else if (grabbable) {
            dynamic_objects.push_back(DynamicObject(index));
        }
    }

    for (int n = 0; n < Node->mNumChildren; n++)
        process_node(Node->mChildren[n], Scene, Node->mTransformation);
}

void Scene::find_instances()
{
    auto it = Meshes.begin();
    auto find_inst = [&it](Mesh& M){return *it == M && !M.m.discard;};
    for(; it < Meshes.end(); it++)
    {
        auto res = it;
        while((res = std::find_if(it+1, Meshes.end(), find_inst)) != Meshes.end())
        {
            Scene::combine_as_instance(*it, *res);
        }
    }
}
void Scene::find_batchable()
{
    auto it = Meshes.begin();
    auto find_batch = [&it](Mesh& M){return it->m.materialIndex == M.m.materialIndex && !M.m.discard;};
    for(; it < Meshes.end(); it++)
    {
        auto res = it;
        while((res = std::find_if(it+1, Meshes.end(), find_batch)) != Meshes.end())
        {
            Scene::combine_meshes(*it, *res);
        }
    }
}

void Scene::load_camera(const aiScene* S)
{
    if(S->HasCameras())
    {
        const auto& aiCam = *S->mCameras[0];
        aiMatrix4x4 Transform = getTransform(aiCam.mName, S);
        aiMatrix3x3 vTransform = aiMatrix3x3(Transform);
        Camera::members& cam = m.defaultCamera.m;
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

void Scene::load_lights(const aiScene* S)
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
        this->Lights[n].color = cast3D(iLight->mColorDiffuse);
        this->Lights[n].direction = glm::normalize(cast3D(aiMatrix3x3(Transform) * iLight->mDirection));
        this->Lights[n].inner_cone = iLight->mAngleInnerCone;
        this->Lights[n].outer_cone = iLight->mAngleOuterCone;
        this->Lights[n].type = (unsigned int)iLight->mType;
    }
}

void Scene::getTexture(aiTextureType Type, const path& parentPath, const aiMaterial* M, MaterialBuffer& matBuff, Material& Mat)
{
    int* texture = nullptr;
    Material::Texture_Index Index;

    switch(Type)
    {
        case aiTextureType_BASE_COLOR:
            texture = &matBuff.Albedo;
            Index = Material::Texture_Index::Albedo;
            break;
        case aiTextureType_NORMALS:
            texture = &matBuff.Normal;
            Index = Material::Texture_Index::Normal;
            break;
        case aiTextureType_DIFFUSE_ROUGHNESS:
            texture = &matBuff.Roughness_metallic;
            Index = Material::Texture_Index::Roughness;
            break;
        case aiTextureType_EMISSIVE:
            texture = &matBuff.Emissive;
            Index = Material::Texture_Index::Emissive;
            break;
        default:
            return;
    }
    if (M->GetTextureCount(Type)){
        aiString texture_path;
        M->GetTexture(Type, 0, &texture_path);
        path P = parentPath / texture_path.C_Str();
        *texture = Mat.load(P, Index).get_slot();
    } else {
        *texture = -1;
    }
}

void Scene::load_materials(const aiScene* S, const path& pFile)
{
    unsigned int nMats = S->mNumMaterials;
    path parentPath = pFile.parent_path();
    for(int n = 0; n < nMats; n++){
        const aiMaterial *M = S->mMaterials[n];
        MaterialBuffer matBuff;
        Material Mat;

        getTexture(aiTextureType_BASE_COLOR, parentPath, M, matBuff, Mat);
        getTexture(aiTextureType_NORMALS, parentPath, M, matBuff, Mat);
        getTexture(aiTextureType_DIFFUSE_ROUGHNESS, parentPath, M, matBuff, Mat);
        getTexture(aiTextureType_EMISSIVE, parentPath, M, matBuff, Mat);
        matBuff.glow = 0;

        m.Materials.push_back(matBuff);
        Materials.push_back(Mat);
    }
}

void Scene::clean_meshes()
{
    Meshes.erase(std::remove_if(Meshes.begin(), Meshes.end(), [](Mesh& m){return m.m.discard;}), Meshes.end());
}

void Scene::set_max_instances()
{
    m.max_instances = 1;
}

Scene* Scene::import_scene(const path& pFile) 
{
    Assimp::Importer importer;

    const aiScene* S = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Quality | aiProcess_GenBoundingBoxes);

    if (!S) {
        std::cerr << "Error importing file '" << pFile << "':\n" << importer.GetErrorString();
        return nullptr;
    }

    Scene *scene = new Scene;

    scene->load_lights(S);

    scene->load_materials(S, pFile);

    scene->load_camera(S);

    scene->Meshes.reserve(S->mRootNode->mNumMeshes);

    scene->process_node(S->mRootNode, S);

    //Scene->find_instances();
    //Scene->find_batchable();
    //Scene->clean_meshes();

    scene->set_max_instances();

    for (StaticObject &obj: scene->static_objects)
    {
        auto &Mesh = scene->Meshes[obj.m.Mesh];
        for(int n = 0; n < Mesh.m.indices.size()/3; n++)
        {
            unsigned int A = Mesh.m.indices[n*3];
            unsigned int B = Mesh.m.indices[n*3+1];
            unsigned int C = Mesh.m.indices[n*3+2];
            triangle T;
            T.A = glm::vec3(Mesh.m.transform * glm::vec4(Mesh.m.vertices[A].position, 1.0f));
            T.B = glm::vec3(Mesh.m.transform * glm::vec4(Mesh.m.vertices[B].position, 1.0f));
            T.C = glm::vec3(Mesh.m.transform * glm::vec4(Mesh.m.vertices[C].position, 1.0f));
            obj.collision_mesh.push_back(T);
        }
    }

    for(auto &Obj : scene->static_objects){
        scene->objects.push_back(&Obj);
    }
    for(auto& Obj : scene->dynamic_objects){
        scene->objects.push_back(&Obj);
    }

    return scene;
}

void Scene::reload_DeferredShader(ShaderStage stage, const std::vector<std::string>& preprocessor_constants)
{
    shader_source src;
    Shader *S = nullptr;
    if (stage == Fragment){
            S = &m.fShader;
            src = shader_source(EShLangFragment, m.deferred_fShader);

    } else if (stage == Vertex){
            S = &m.vShader;
            src = shader_source(EShLangVertex, m.deferred_vShader);
    }
    std::string preamble = "";
    for (auto &S : preprocessor_constants){
            preamble += S + "\n";
    }
    std::vector<unsigned int> SPIRV;
    compile_shader_SPV(src, SPIRV, preamble.c_str());
    m.sProgram.dettachShader(*S);
    S->reloadBinary(SPIRV);
    S->specialize();
    m.sProgram.attachShader(*S);
    m.sProgram.linkProgram();
}
void Scene::reload_DeferredShader(ShaderStage stage)
{
    shader_source src;
    Shader *S = nullptr;
    if (stage == Fragment){
            S = &m.fShader;
            src = shader_source(EShLangFragment, m.deferred_fShader);

    } else if (stage == Vertex){
            S = &m.vShader;
            src = shader_source(EShLangVertex, m.deferred_vShader);
    }
    std::vector<unsigned int> SPIRV;
    compile_shader_SPV(src, SPIRV);
    m.sProgram.dettachShader(*S);
    S->reloadBinary(SPIRV);
    S->specialize();
    m.sProgram.attachShader(*S);
    m.sProgram.linkProgram();
}

void Scene::load_DeferredShader(const path& vertex, const path& fragment)
{
    m.deferred_vShader = vertex;
    m.deferred_fShader = fragment;

    m.vShader = compile_shader(shader_source(EShLanguage::EShLangVertex, m.deferred_vShader));
    m.fShader = compile_shader(shader_source(EShLanguage::EShLangFragment, m.deferred_fShader));

    //m.vShader.loadBinary(vertex);
    //m.fShader.loadBinary(fragment);

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

void Scene::updatePostProcessBuffer()
{
    m.postProcess_buffer.bufferData(sizeof(postprocess), &postprocess, GL_DYNAMIC_DRAW);
}

void Scene::setUBO()
{
    m.UBO.generate();
    m.UBO.bufferData(Lights.size()*sizeof(Light), Lights.data(), GL_DYNAMIC_DRAW);
    m.UBO.bindBufferBase(2);

    m.mBuffer.generate();
    m.mBuffer.bufferData(sizeof(MaterialBuffer), nullptr, GL_DYNAMIC_DRAW);
    m.mBuffer.bindBufferBase(3);

    m.postProcess_buffer.generate();
    m.postProcess_buffer.bufferData(sizeof(postprocess), &postprocess, GL_DYNAMIC_DRAW);
    m.postProcess_buffer.bindBufferBase(4);
}

void Scene::render_static_objects(Buffer &Transform_Buffer, size_t transform_offset)
{
    for (auto &Obj : static_objects)
    {
        auto& M = Meshes[Obj.m.Mesh];
        if (!M.m.visible)
            continue;
        m.mBuffer.bufferSubData(sizeof(MaterialBuffer), 0, &m.Materials[M.m.materialIndex]);
        Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &M.m.transform);
        M.render();
    }
}

void Scene::render_dynamic_objects(Buffer &Transform_Buffer, size_t transform_offset)
{
    for (auto &Obj : dynamic_objects)
    {
        auto& M = Meshes[Obj.m.Mesh];
        if (!M.m.visible)
            continue;
        if (Obj.m_grabbed)
        {
            m.Materials[M.m.materialIndex].alpha = 0.5f;
        }
        else
        {
            m.Materials[M.m.materialIndex].alpha = 1.0f;
        }

        m.mBuffer.bufferSubData(sizeof(MaterialBuffer), 0, &m.Materials[M.m.materialIndex]);
        if (Obj.m_selected)
        {
            glCullFace(GL_FRONT);
            int glow = 1;
            m.mBuffer.bufferSubData(4UL, offsetof(MaterialBuffer, glow), &glow);
            glm::mat4 T = glm::scale(M.m.transform, glm::vec3(1.1f));
            Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &T);
            M.render();
            glow = 0;
            m.mBuffer.bufferSubData(4UL, offsetof(MaterialBuffer, glow), &glow);
            glCullFace(GL_BACK);
        }
        Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &M.m.transform);
        M.render();
    }
}

void Scene::render_postprocess()
{
    Framebuffer::unbind();
    m.PostProcRenderer.use_shader();
    m.DRenderer.set_viewport();
    m.PostProcRenderer.render();
}

void Scene::render_deferred()
{
    m.PostProcRenderer.bind_framebuffer();
    m.sProgram.use();
    m.PostProcRenderer.set_viewport();
    m.DRenderer.render();
}

void Scene::bind_materials()
{
    for(auto& M : Materials)
        M.bind();
}

void Scene::render_skybox()
{
    if (!m.skybox)
        return;
    
    m.skybox.use_shader();
    m.skybox.bind_texture();
    m.skybox.render();
}

void Scene::render(Buffer &Transform_Buffer, size_t transform_offset)
{
    m.DRenderer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bind_materials();
    render_static_objects(Transform_Buffer, transform_offset);
    render_dynamic_objects(Transform_Buffer, transform_offset);
    render_skybox();
    render_deferred();
    render_postprocess();
}

void Scene::initSkybox(const path& vertex, const path& fragment, const path& skybox)
{
    m.skybox.init(vertex, fragment, 512, 512, skybox);
}

void Scene::initMeshes()
{
    for(auto& M : Meshes)
        M.init();
}

void Scene::initDRenderer(int w, int h, int MSAA)
{
    m.DRenderer.initDeferred(m.scene_vShader, m.scene_fShader, m.Materials.size()*4, w, h, MSAA);
    m.PostProcRenderer.initPostProc(m.postProcess_vShader, m.postProcess_fShader, w,h);
}
void Scene::initDRenderer(int w, int h)
{
    m.DRenderer.initDeferred(m.scene_vShader, m.scene_fShader, m.Materials.size()*4, w, h);
    m.PostProcRenderer.initPostProc(m.postProcess_vShader, m.postProcess_fShader, w,h);
}
