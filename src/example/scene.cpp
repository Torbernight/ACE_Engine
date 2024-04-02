#include <GL/glew.h>
#include <thread>
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
        
        aiString AlbedoPath, NormalPath, RoughnessPath;
        M->GetTexture(aiTextureType_BASE_COLOR, 0, &AlbedoPath);
        M->GetTexture(aiTextureType_NORMALS, 0, &NormalPath);
        M->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &RoughnessPath);
        if (AlbedoPath.length == 0 || NormalPath.length == 0 || RoughnessPath.length == 0)
            continue;
        Material Mat;
        Mat.load(pFile.parent_path() / AlbedoPath.C_Str(), pFile.parent_path() / NormalPath.C_Str(), pFile.parent_path() / RoughnessPath.C_Str());
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
        mat.Albedo = M.get_albedo().get_slot();
        mat.Normal = M.get_normal().get_slot();
        mat.Roughness = M.get_roughness().get_slot();
        m.Materials.push_back(mat);
    }

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
