#include <GL/glew.h>
#include "buffer.hpp"
#include "vertexArray.hpp"

void vertexArray::bind()
{
    glBindVertexArray(m.VAO);
}


void vertexArray::unbind()
{
    glBindVertexArray(0);
}


void vertexArray::generateVAO()
{
    glGenVertexArrays(1, &m.VAO);
}

void vertexArray::setVertexAttributes(const Buffer &B, size_t offset, size_t stride)
{
    bind();

    glBindVertexBuffer(0, B.getBufferObject(), offset, stride);

    for(int n = 0; n < m.nComponents; n++)
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
                glVertexAttribFormat(Attrib.index, Attrib.size, Attrib.type, Attrib.normalized, Attrib.offset);
                break;

            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
                glVertexAttribIFormat(Attrib.index, Attrib.size, Attrib.type, Attrib.offset);
                break;
        }

        glVertexAttribBinding(Attrib.index, 0);
    }

    unbind();
}

void vertexArray::setVertexAttributes(Buffer &B)
{
    bind();

    B.bind();

    for(int n = 0; n < m.nComponents; n++)
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
                glVertexAttribPointer(Attrib.index, Attrib.size, Attrib.type, Attrib.normalized, Attrib.stride, (void*)Attrib.offset);
                break;

            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
                glVertexAttribIPointer(Attrib.index, Attrib.size, Attrib.type, Attrib.stride, (void*)Attrib.offset);
                break;
        }

    }

    unbind();
}


vertexArray::~vertexArray()
{
    if (m.VAO)
        glDeleteVertexArrays(1, &m.VAO);
}
