#include <GL/glew.h>
#include "buffer.hpp"

namespace Renderer {


Buffer::Buffer(int target)
: m(target)
{
}

void Buffer::generate()
{
    glGenBuffers(1, &m.BufferObject);
}

void Buffer::bind()
{
    glBindBuffer(m.target, m.BufferObject);
}

void Buffer::unbind()
{
    glBindBuffer(m.target, 0);
}

void Buffer::bindBufferBase(int binding)
{
    m.binding = binding;
    glBindBufferBase(m.target, m.binding, m.BufferObject);
}

void Buffer::bindBufferBase()
{
    glBindBufferBase(m.target, m.binding, m.BufferObject);
}

void Buffer::bufferData(int size, const void* data, int usage)
{
    m.size = size;
    bind();
    glBufferData(m.target, m.size, data, usage);
    unbind();
}

void Buffer::bufferData(const void* data, int usage)
{
    bind();
    glBufferData(m.target, m.size, data, usage);
    unbind();
}


void Buffer::bufferSubData(int size, int offset, const void* data)
{
    bind();
    glBufferSubData(m.target, offset, size, data);
    unbind();
}

Buffer::~Buffer()
{
    if (m.BufferObject)
        glDeleteBuffers(1, &m.BufferObject);
}

}