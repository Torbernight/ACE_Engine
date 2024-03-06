#include <GL/glew.h>
#include "buffer.hpp"

buffer::buffer(int target)
: m(target)
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
    glNamedBufferData(m.BufferObject, m.size, data, usage);
}

void buffer::bufferData(const void* data, int usage)
{
    glNamedBufferData(m.BufferObject, m.size, data, usage);
}


void buffer::bufferSubData(int size, int offset, const void* data)
{
    glNamedBufferSubData(m.BufferObject, offset, size, data);
}