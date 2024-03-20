#pragma once

#include <vector>
#include <initializer_list>

class buffer;

class vertexArray
{
    public:
    struct vertexAttrib
    {
        unsigned index;
        int size;
        int type;
        int normalized;
        int stride;
        unsigned int offset;
    };

    //vertexAttrib Attributes[nComponents];
    std::vector<vertexAttrib> Attributes;

    void generateVAO();

    void setVertexAttributes(buffer &B);
    void setVertexAttributes(const buffer &B, size_t offset, size_t stride);

    void bind();

    static void unbind();

    ~vertexArray();

    vertexArray(unsigned int nComponents)
    : m({0U, nComponents})
    {
        Attributes.resize(nComponents);
    }

    vertexArray(const std::vector<vertexAttrib>& vertex_spec)
    : Attributes(vertex_spec) {}

    vertexArray(std::initializer_list<vertexAttrib>&& vertex_spec)
    : Attributes(vertex_spec) {}

    unsigned int getnComponents() const {return m.nComponents;} 

    private:
    struct members {
        unsigned int VAO = 0;
        unsigned int nComponents = 3;
    } m;

};
