#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <assimp/color4.h>

struct vector2;
struct vector3;
struct vector4;

struct vector2{
        float x,y;
        
        vector2(const aiVector2D& v): x(v.x), y(v.y)
        {}
        vector2(const aiVector3D& v): x(v.x), y(v.y)
        {}
        vector2(const aiColor4D& v): x(v.r), y(v.g)
        {}
        vector2(const glm::vec2& v) : x(v.x), y(v.y)
        {}
        vector2(const glm::vec3& v) : x(v.x), y(v.y)
        {}
        vector2(const glm::vec4& v) : x(v.x), y(v.y)
        {}
        vector2(): x(0), y(0) 
        {}
        vector2(float _x, float _y): x(_x), y(_y)
        {}
        vector2(float k): x(k), y(k)
        {}

        inline vector2(const vector3 &v);

        operator glm::vec2(){
                return glm::vec2(x,y);
        }
        operator aiVector2D(){
                return aiVector2D(x,y);
        }
};
struct vector3{
        float x,y,z;

        vector3(const aiVector3D& v): x(v.x), y(v.y), z(v.z)
        {}
        vector3(const aiColor4D& v): x(v.r), y(v.g), z(v.b)
        {}
        vector3(const glm::vec3& v): x(v.x), y(v.y), z(v.z)
        {}
        vector3(const glm::vec4& v): x(v.x), y(v.y), z(v.z)
        {}
        vector3(): x(0), y(0), z(0)
        {}
        vector3(float _x, float _y, float _z): x(_x), y(_y), z(_z)
        {}
        vector3(float k): x(k), y(k), z(k)
        {}

        inline vector3(const vector4&);

        operator glm::vec3(){
                return glm::vec3(x,y,z);
        }
        operator aiVector3D(){
                return aiVector3D(x,y,z);
        }
};


struct vector4{
        float x,y,z,w;

        vector4(const aiColor4D& v): x(v.r), y(v.g), z(v.b), w(v.a)
        {}        
        vector4(const glm::vec4& v): x(v.x), y(v.y), z(v.z), w(v.w)
        {}
        vector4(): x(0), y(0), z(0), w(0)
        {}
        vector4(float _x, float _y, float _z, float _w): x(_x), y(_y), z(_z), w(_w)
        {}
        vector4(float k): x(k), y(k), z(k), w(k)
        {}

        operator glm::vec4(){
                return glm::vec4(x,y,z,w);
        }
        operator aiColor4D(){
                return aiColor4D(x,y,z,w);
        }
};

inline vector2::vector2(const vector3 &v): x(v.x), y(v.y)
{}
inline vector3::vector3(const vector4& v) : x(v.x), y(v.y), z(v.z)
{}