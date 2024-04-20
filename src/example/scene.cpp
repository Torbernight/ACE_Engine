#include <GL/glew.h>
#include <thread>
#include <algorithm>
#include <execution>
#include "scene.hpp"
#include "scene_helper.hpp"

void scene::combine_as_instance(mesh& A, mesh& Instance)
{
    //A.m.nInstances += Instance.m.nInstances;
    //for (int n = 0; n < Instance.m.nInstances; n++)
    //{
    //    A.m.transforms.push_back(Instance.m.transforms[n]);
    //}
    //Instance.m.discard = true;
}

void scene::combine_meshes(mesh& A, mesh& B)
{
    mesh AB;
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
    AB.m.Name = A.m.Name;
    AB.m.transform = glm::mat4(1.0f);
    AB.join_identical_vertices();
    B.m.discard = true;
    A = AB;
}

mesh scene::process_mesh(const aiMesh* Mesh, const aiMatrix4x4& Transform)
{
    mesh M;
    M.m.Name = Mesh->mName.C_Str();

    M.m.vertices.reserve(Mesh->mNumVertices);
    M.m.indices.reserve(Mesh->mNumFaces*3);

    std::thread Tfaces(process_faces, Mesh, &M.m.indices);
    std::thread Tverts(process_vertices, Mesh, &M.m.vertices);

    M.m.bounding_box.max = cast3D(Mesh->mAABB.mMax);
    M.m.bounding_box.min = cast3D(Mesh->mAABB.mMin);
    M.m.transform = cast4x4(Transform);
    M.m.materialIndex = Mesh->mMaterialIndex;

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

void scene::process_node(const aiNode* Node, const aiScene* Scene)
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
            static_objects.push_back(static_object(index));
        } else if (grabbable) {
            dynamic_objects.push_back(dynamic_object(index));
        }
    }

    for (int n = 0; n < Node->mNumChildren; n++)
        process_node(Node->mChildren[n], Scene, Node->mTransformation);
}
#undef XYZ

void scene::process_node(const aiNode* Node, const aiScene* Scene, const aiMatrix4x4 &ParentTransform)
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
            static_objects.push_back(static_object(index));
        } else if (grabbable) {
            dynamic_objects.push_back(dynamic_object(index));
        }
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
    auto find_inst = [&it](mesh& M){return *it == M && !M.m.discard;};
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
    auto find_batch = [&it](mesh& M){return it->m.materialIndex == M.m.materialIndex && !M.m.discard;};
    for(; it < Meshes.end(); it++)
    {
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
        this->Lights[n].color = cast3D(iLight->mColorDiffuse);
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
            break;
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
        matBuff.glow = 0;

        m.Materials.push_back(matBuff);
        Materials.push_back(Mat);
    }
}

void scene::clean_meshes()
{
    Meshes.erase(std::remove_if(Meshes.begin(), Meshes.end(), [](mesh& m){return m.m.discard;}), Meshes.end());
}

void scene::set_max_instances()
{
    m.max_instances = 1;
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

    Scene->Meshes.reserve(S->mRootNode->mNumMeshes);

    Scene->process_node(S->mRootNode, S);

    //Scene->find_instances();
    //Scene->find_batchable();
    //Scene->clean_meshes();

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

void scene::render_static_objects(buffer &Transform_Buffer, size_t transform_offset)
{
    for (auto &Obj : static_objects)
    {
        auto& M = Meshes[Obj.m.Mesh];
        if (!M.m.visible)
            continue;
        m.mBuffer.bufferSubData(sizeof(material_buffer), 0, &m.Materials[M.m.materialIndex]);
        Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &M.m.transform);
        M.render();
    }
}

void scene::render_dynamic_objects(buffer &Transform_Buffer, size_t transform_offset)
{
    for (auto &Obj : dynamic_objects)
    {
        auto& M = Meshes[Obj.m.Mesh];
        if (!M.m.visible)
            continue;
        m.mBuffer.bufferSubData(sizeof(material_buffer), 0, &m.Materials[M.m.materialIndex]);
        if (Obj.m_selected)
        {
            glCullFace(GL_FRONT);
            int glow = 1;
            m.mBuffer.bufferSubData(4UL, offsetof(material_buffer, glow), &glow);
            glm::mat4 T = glm::scale(M.m.transform, glm::vec3(1.1f));
            Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &T);
            M.render();
            glow = 0;
            m.mBuffer.bufferSubData(4UL, offsetof(material_buffer, glow), &glow);
            glCullFace(GL_BACK);
        }
        Transform_Buffer.bufferSubData(sizeof(glm::mat4), transform_offset, &M.m.transform);
        M.render();
    }
}

void scene::render(buffer &Transform_Buffer, size_t transform_offset)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m.DRenderer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(auto& M : Materials)
        M.bind();

    render_static_objects(Transform_Buffer, transform_offset);
    render_dynamic_objects(Transform_Buffer, transform_offset);

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
