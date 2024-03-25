#include <GL/glew.h>
#include "texture.hpp"

texture::~texture()
{
        glDeleteTextures(1, &m.texture);
}

void texture::generate()
{
        glGenTextures(1, &m.texture);
}

void texture::bind_unit()
{
        glBindTextureUnit(m.slot, m.texture);
}

void texture::bind()
{
        glBindTexture(m.target, m.texture);
}
void texture::unbind()
{
        glBindTexture(m.target, 0);
}

void texture::genTexImage(const void* pixels)
{
        switch(m.target)
        {
                case GL_TEXTURE_1D:
                case GL_PROXY_TEXTURE_1D:
                glTexImage1D(m.target, 0, m.internal_format, m.width, 0, m.format, m.type, pixels);
                break;

                case GL_TEXTURE_2D: 
                case GL_PROXY_TEXTURE_2D: 
                case GL_TEXTURE_1D_ARRAY: 
                case GL_PROXY_TEXTURE_1D_ARRAY: 
                case GL_TEXTURE_RECTANGLE: 
                case GL_PROXY_TEXTURE_RECTANGLE: 
                case GL_TEXTURE_CUBE_MAP_POSITIVE_X: 
                case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: 
                case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: 
                case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: 
                case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: 
                case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: 
                case GL_PROXY_TEXTURE_CUBE_MAP:
                glTexImage2D(m.target, 0, m.internal_format, m.width, m.height, 0, m.format, m.type, pixels);
                break;

                case GL_TEXTURE_3D:
                case GL_PROXY_TEXTURE_3D:
                case GL_TEXTURE_2D_ARRAY:
                case GL_PROXY_TEXTURE_2D_ARRAY:
                glTexImage3D(m.target, 0, m.internal_format, m.width, m.height, m.depth, 0, m.format, m.type, pixels);

        }
}

void texture::setTexParameter(unsigned int parameter, int value)
{
        glTexParameteri(m.target, parameter, value);
}
void texture::setTexParameter(unsigned int parameter, float value)
{
        glTexParameterf(m.target, parameter, value);
}

void texture::setTexParameter(unsigned int parameter, const float *value)
{
        glTexParameterfv(m.target, parameter, value);
}
void texture::setTexParameter(unsigned int parameter, const int *value)
{
        glTexParameteriv(m.target, parameter, value);
}
