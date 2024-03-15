#pragma once

#include <vector>

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
        void* pointer;
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

    unsigned int getnComponents() const {return m.nComponents;} 

    private:
    struct members {
        unsigned int VAO = 0;
        unsigned int nComponents = 3;
    } m;

};
