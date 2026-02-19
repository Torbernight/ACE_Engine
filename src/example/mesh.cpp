#include <GL/glew.h>
#include <cfloat>
#include <algorithm>
#include <execution>
#include "mesh.hpp"

void Mesh::init()
{
    m.VBO.generate();
    m.VBO.bufferData(m.vertices.size()*sizeof(Vertex), m.vertices.data(), GL_STATIC_DRAW);
    m.VAO.generateVAO();
    m.VAO.setVertexAttributes(m.VBO);
    m.VAO.bind();
    m.EBO.generate();
    m.EBO.bind();
    m.EBO.bufferData(m.indices.size()*4UL, m.indices.data(), GL_STATIC_DRAW);
    m.VAO.unbind();
}

void Mesh::apply_transform()
{
    using namespace glm;
    mat3 ntransform = transpose(inverse(mat3(m.transform)));
    for(auto &V : m.vertices)
    {
        V.position = vec3(m.transform * vec4(V.position, 1.0));
        V.normal = ntransform * V.normal;
        V.tangent = ntransform * V.tangent;
        V.bitangent = ntransform * V.bitangent;
    }
    //if (m.nInstances > 1) {
    //    mat4 Inv = inverse(m.transforms[0]);
    //    for (auto& T : m.transforms) {
    //        T = Inv * T;
    //    }
    //} else {
        m.transform = mat4(1.0f);
    //}
    compute_AABB();
}

void Mesh::join_identical_vertices()
{
    std::vector<unsigned int> repeated;
    
    for(auto it = m.vertices.begin(); it < m.vertices.end(); it++)
    {
        if (0 == std::count(it+1, m.vertices.end(), *it))
            continue;

        unsigned int index = it - m.vertices.begin();
        auto res = it;

        while((res = std::find(res+1, m.vertices.end(), *it)) != m.vertices.end())
        {
            unsigned int old_index = res - m.vertices.begin();
            repeated.push_back(old_index);
            std::replace(m.indices.begin(), m.indices.end(), old_index, index);
        }
    }
    if (!repeated.empty())
        m.vertices.erase(std::remove_if(m.vertices.begin(), m.vertices.end(), [&](Vertex &V){return std::count(repeated.begin(), repeated.end(), (unsigned int)(&V - &m.vertices[0])) > 0;}));
}

void Mesh::render()
{
    if (!m.visible)
        return;
    m.VAO.bind();
    m.EBO.bind();
    glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_INT, nullptr);
}

void Mesh::onFrustum(const frustum& F)
{
    m.visible = F.testAABB(m.bounding_box*m.transform);
}

bool Mesh::isInstance(const Mesh& M) const
{
    static const std::regex pattern("\\.[0-9][0-9][0-9]");

    std::string subName0 = std::regex_replace(m.name,pattern,"");
    std::string subName1 = std::regex_replace(M.m.name,pattern,"");
    
    return subName0 == subName1 &&
        m.vertices.size() == M.m.vertices.size() &&
        m.indices.size() == M.m.indices.size() &&
        m.materialIndex == M.m.materialIndex;
}

void Mesh::compute_AABB()
{
    glm::vec3 maxV(-FLT_MAX), minV(FLT_MAX);
    for(unsigned int index : m.indices)
    {
        if (UINT32_MAX == index)
            continue;
        maxV.x = glm::max(m.vertices[index].position.x, maxV.x);
        maxV.y = glm::max(m.vertices[index].position.y, maxV.y);
        maxV.z = glm::max(m.vertices[index].position.z, maxV.z);

        minV.x = glm::min(m.vertices[index].position.x, minV.x);
        minV.y = glm::min(m.vertices[index].position.y, minV.y);
        minV.z = glm::min(m.vertices[index].position.z, minV.z);
    }
    
    m.bounding_box = AABB(minV, maxV);
}