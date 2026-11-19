#pragma once

namespace Renderer 
{
class Buffer
{
    public:

    Buffer(int target);

    Buffer(const Buffer& B) :
    m(B.m)
    {
        const_cast<Buffer&>(B).m = {};
    }
    Buffer& operator=(const Buffer& B)
    {
        Buffer temp(B);
        m = temp.m;
        temp.m = {};
        return *this;
    }

    ~Buffer();

    void generate();

    void bufferData(int size, const void* data, int usage);
    void bufferData(const void* data, int usage);
    void bufferSubData(int size, int offset, const void* data);
    void bindBufferBase(int binding);
    void bindBufferBase();

    void bind();
    void unbind();

    unsigned int getBufferObject() const {return m.BufferObject;}

    private:

    struct members
    {
        unsigned int BufferObject = 0;
        int target = 0;
        int size = 0;
        int binding = 0;

        members() = default;
        members(int Target) : target(Target) {}
        members(int Size, int Target) :  target(Target), size(Size) {}
        members(int Size, int Binding, int Target) :  target(Target), size(Size), binding(Binding) {}
    } m;
};
}