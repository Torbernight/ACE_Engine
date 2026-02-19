#pragma once

#include <vector>
#include <initializer_list>

class Buffer;

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

    std::vector<vertexAttrib> Attributes;

    void generateVAO();

    void setVertexAttributes(Buffer &B);
    void setVertexAttributes(const Buffer &B, size_t offset, size_t stride);

    void bind();

    static void unbind();

    ~vertexArray();

    vertexArray(unsigned int nComponents)
    : m({0U, nComponents})
    {
        Attributes.resize(nComponents);
    }

    vertexArray(const std::vector<vertexAttrib>& vertex_spec)
    : Attributes(vertex_spec) {
        m.nComponents = Attributes.size();
    }

    vertexArray(std::initializer_list<vertexAttrib>&& vertex_spec)
    : Attributes(vertex_spec) {
        m.nComponents = Attributes.size();
    }

    vertexArray(const vertexArray& V) :
    Attributes(V.Attributes), m(V.m)
    {
        auto& v = const_cast<vertexArray&>(V);
        v.m = {};
    }

    vertexArray& operator=(const vertexArray& V)
    {
        vertexArray temp(V);
        m = temp.m;
        Attributes = temp.Attributes;
        temp.m = {};
        return *this;
    }

    unsigned int getnComponents() const {return m.nComponents;} 

    private:
    struct members {
        unsigned int VAO = 0;
        unsigned int nComponents = 3;
    } m;

};
