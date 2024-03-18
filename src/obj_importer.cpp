#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <glm/geometric.hpp>
#include "obj_importer.hpp"


void obj::gen_index()
{
    std::vector<vertex> unique_vertices;
    unique_vertices.reserve(m.vertices.size());
    unsigned int index = 0;
    for(unsigned int n = 0; n < m.vertices.size(); n++)
    {
        vertex &V = m.vertices[n];
        auto equal = [V](const vertex& v){return V.pos == v.pos && V.normal == v.normal && V.texcoord == v.texcoord;};
        auto it = std::find_if(unique_vertices.begin(), unique_vertices.end(), equal);
        if (it == unique_vertices.end())
        {
            unique_vertices.push_back(V);
            m.indices.push_back(index++);
            continue;
        }
        unsigned int i = it - unique_vertices.begin();
        m.indices.push_back(i);
    }
    m.vertices = unique_vertices;
}

void obj::smooth_normals()
{
    std::vector<glm::vec3> unique_positions;
    unique_positions.reserve(m.vertices.size());
    for(unsigned int n = 0; n < m.vertices.size(); n++)
    {
        vertex &V = m.vertices[n];
        if (std::find(unique_positions.begin(), unique_positions.end(), V.pos) == unique_positions.end())
        {
            unique_positions.push_back(V.pos);
        }
    }
    std::vector<vertex*> verts;
    std::vector<glm::vec3> normals;

    for (auto &P : unique_positions)
    {
        auto it = std::find_if(m.vertices.begin(), m.vertices.end(), [P](const vertex &v){return v.pos == P;});
        while(it < m.vertices.end())
        {
            verts.push_back(&(*it));
            normals.push_back(it->normal);
            it = std::find_if(it+1, m.vertices.end(), [P](const vertex &v){return v.pos == P;});
        }
        if(verts.size() <= 1){
            verts.clear();
            normals.clear();
            continue;
        }

        glm::vec3 normal = glm::vec3(0);
        for(const auto& N : normals)
            normal += N;
        normal = glm::normalize(normal);
        for(vertex* V: verts)
            V->normal = normal;

        verts.clear();
        normals.clear();

    }

}

void obj::import(const std::filesystem::path& P)
{
    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> TexCoords;

    std::ifstream file(P.c_str());
    std::string line;
    char c[64];
    float x,y,z;

    while(std::getline(file, line))
    {
        std::stringstream ss(line);
        ss >> c;

        if (!strcmp(c, "#"))
            continue;
        if (!strcmp(c, "mtllib"))
            continue;
        if (!strcmp(c, "s"))
            continue;

        if (!strcmp(c, "v"))
        {
            ss >> x >> y >> z;
            glm::vec3 V(x,y,z);
            Positions.push_back(V);
        }

        if (!strcmp(c, "vn"))
        {
            ss >> x >> y >> z;
            glm::vec3 V(x,y,z);
            Normals.push_back(glm::normalize(V));
        }
        if (!strcmp(c, "vt"))
        {
            ss >> x >> y;
            glm::vec2 V(x,y);
            TexCoords.push_back(V);
        }
        if (!strcmp(c, "f"))
        {
            unsigned int Pindex, Nindex, Tindex;
            char trash;
            for(int n = 0; n < 3; n++){
                ss >> Pindex >> trash  >> Tindex >> trash >> Nindex;
                addVertex(Positions[Pindex-1], TexCoords[Tindex-1], Normals[Nindex-1]);
            }
        }
    }
    file.close();
}