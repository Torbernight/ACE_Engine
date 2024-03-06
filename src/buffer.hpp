#pragma once

class buffer
{
    public:

    buffer(int target);

    ~buffer();

    void bufferData(int size, const void* data, int usage);
    void bufferData(const void* data, int usage);
    void bufferSubData(int size, int offset, const void* data);
    void bindBufferBase(int binding);
    void bindBufferBase();

    void bind();
    void unbind();

    private:

    struct members
    {
        unsigned int BufferObject = 0;
        int target = 0;
        int size = 0;
        int binding = 0;

        members(int Target) : target(Target) {}
        members(int Size, int Target) :  target(Target), size(Size) {}
        members(int Size, int Binding, int Target) :  target(Target), size(Size), binding(Binding) {}
    } m;
};