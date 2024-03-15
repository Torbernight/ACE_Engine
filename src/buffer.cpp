#include <GL/glew.h>
#include "buffer.hpp"

buffer::buffer(int target)
: m(target)
{
}

void buffer::generate()
{
    glGenBuffers(1, &m.BufferObject);
}

void buffer::bind()
{
    glBindBuffer(m.target, m.BufferObject);
}

void buffer::unbind()
{
    glBindBuffer(m.target, 0);
}

void buffer::bindBufferBase(int binding)
{
    m.binding = binding;
    glBindBufferBase(m.target, m.binding, m.BufferObject);
}

void buffer::bindBufferBase()
{
    glBindBufferBase(m.target, m.binding, m.BufferObject);
}

void buffer::bufferData(int size, const void* data, int usage)
{
    m.size = size;
    bind();
    glBufferData(m.target, m.size, data, usage);
    unbind();
}

void buffer::bufferData(const void* data, int usage)
{
    bind();
    glBufferData(m.target, m.size, data, usage);
    unbind();
}


void buffer::bufferSubData(int size, int offset, const void* data)
{
    bind();
    glBufferSubData(m.target, offset, size, data);
    unbind();
}

buffer::~buffer()
{
    if (m.BufferObject)
        glDeleteBuffers(1, &m.BufferObject);
}