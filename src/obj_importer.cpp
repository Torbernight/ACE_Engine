#include <fstream>
#include <iostream>
#include <cstring>
#include <glm/geometric.hpp>
#include "obj_importer.hpp"

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
            int Pindex, Nindex, Tindex;
            char trash;
            for(int n = 0; n < 3; n++){
                ss >> Pindex >> trash  >> Nindex >> trash >> Tindex;
                addVertex(Positions[Pindex-1], TexCoords[Tindex-1], Normals[Nindex-1]);
            }
        }
    }
    file.close();
}