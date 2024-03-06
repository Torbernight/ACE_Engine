#pragma once

template<unsigned int nComponents = 3U> 
class vertexArray
{
    static_assert(nComponents > 0, "Error: Vertex array number of components must be greater than 0");

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

    //std::array<vertexAttrib, nComponents> Attributes;
    vertexAttrib Attributes[nComponents];

    void createVAO();

    void bind();

    static void unbind();

    ~vertexArray();

    static unsigned int getnComponents() const {return s_nComponents;} 

    private:
    struct members {
        unsigned int VAO = 0;
    } m;

    inline static constexpr unsigned int s_nComponents = nComponents;

};