#include <GL/glew.h>
#include "vertexArray.hpp"

template<unsigned int nComponents> 
void vertexArray<nComponents>::bind()
{
    glBindVertexArray(m.VAO);
}

template<unsigned int nComponents> 
void vertexArray<nComponents>::unbind()
{
    glBindVertexArray(0);
}

template<unsigned int nComponents> 
void vertexArray<nComponents>::createVAO()
{
    glGenVertexArrays(1, &m.VAO);
    bind();

    for(int n = 0; n < nComponents; n++)
    {
        const vertexAttrib& Attrib = Attributes[n];

        glEnableVertexAttribArray(Attrib.index);

        switch(Attrib.type) {
            case GL_FLOAT:
            case GL_HALF_FLOAT:
            case GL_DOUBLE:
            case GL_FIXED:
            case GL_INT_2_10_10_10_REV:
            case GL_UNSIGNED_INT_2_10_10_10_REV:
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
                glVertexAttribPointer(Attrib.index, Attrib.size, Attrib.type, Attrib.normalized, Attrib.stride, Attrib.pointer);
                break;

            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
                glVertexAttribIPointer(Attrib.index, Attrib.size, Attrib.type, Attrib.stride, Attrib.pointer);
                break;
        }

    }

    unbind();
}