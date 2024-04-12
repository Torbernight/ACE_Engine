#include <GL/glew.h>
#include <cfloat>
#include <algorithm>
#include <execution>
#include "mesh.hpp"

void mesh::init()
{
    VBO.generate();
    VBO.bufferData(vertices.size()*sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
    VAO.generateVAO();
    VAO.setVertexAttributes(VBO);
    VAO.bind();
    EBO.generate();
    EBO.bind();
    EBO.bufferData(indices.size()*4UL, indices.data(), GL_STATIC_DRAW);
    VAO.unbind();
}

void mesh::apply_transform()
{
    using namespace glm;
    mat3 ntransform = transpose(inverse(mat3(transforms[0])));
    for(auto &V : vertices)
    {
        V.position = vec3(transforms[0] * vec4(V.position, 1.0));
        V.normal = ntransform * V.normal;
        V.tangent = ntransform * V.tangent;
        V.bitangent = ntransform * V.bitangent;
    }
    if (nInstances > 1) {
        mat4 Inv = inverse(transforms[0]);
        for (auto& T : transforms) {
            T = Inv * T;
        }
    } else {
        transforms[0] = mat4(1.0f);
    }
    compute_AABB();
}

void mesh::join_identical_vertices()
{
    std::vector<unsigned int> repeated;
    
    for(auto it = vertices.begin(); it < vertices.end(); it++)
    {
        if (0 == std::count(it+1, vertices.end(), *it))
            continue;

        unsigned int index = it - vertices.begin();
        auto res = it;

        while((res = std::find(res+1, vertices.end(), *it)) != vertices.end())
        {
            unsigned int old_index = res - vertices.begin();
            repeated.push_back(old_index);
            std::replace(indices.begin(), indices.end(), old_index, index);
        }
    }
    if (!repeated.empty())
        vertices.erase(std::remove_if(vertices.begin(), vertices.end(), [&](vertex &V){return std::count(repeated.begin(), repeated.end(), (unsigned int)(&V - &vertices[0])) > 0;}));
}

void mesh::render()
{
    if (!nVisibleInstances)
        return;
    VAO.bind();
    EBO.bind();
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, nVisibleInstances);
}

void mesh::onFrustum(const frustum& F)
{
    if (nInstances == 1) {
        nVisibleInstances = F.testAABB(bounding_box*transforms[0]);
        return;
    }
    auto predicate = [F, this](const glm::mat4& T){
        return F.testAABB(bounding_box*T);
    };

    auto middle = std::stable_partition(std::execution::par, transforms.begin(), transforms.end(), predicate);
    nVisibleInstances = middle - transforms.begin();
}

bool mesh::isInstance(const mesh& M) const
{
    static const std::regex pattern("\\.[0-9][0-9][0-9]");

    std::string subName0 = std::regex_replace(Name,pattern,"");
    std::string subName1 = std::regex_replace(M.Name,pattern,"");
    
    return subName0 == subName1 &&
        vertices.size() == M.vertices.size() &&
        indices.size() == M.indices.size() &&
        materialIndex == M.materialIndex;
}

void mesh::compute_AABB()
{
    glm::vec3 maxV(-FLT_MAX), minV(FLT_MAX);
    for(unsigned int index : indices)
    {
        if (UINT32_MAX == index)
            continue;
        maxV.x = glm::max(vertices[index].position.x, maxV.x);
        maxV.y = glm::max(vertices[index].position.y, maxV.y);
        maxV.z = glm::max(vertices[index].position.z, maxV.z);

        minV.x = glm::min(vertices[index].position.x, minV.x);
        minV.y = glm::min(vertices[index].position.y, minV.y);
        minV.z = glm::min(vertices[index].position.z, minV.z);
    }
    
    bounding_box = AABB(minV, maxV);
}