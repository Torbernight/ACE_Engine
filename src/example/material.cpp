#include <GL/glew.h>
#include "material.hpp"

int Material::last_slot = 0;


image_data Material::process_image_data(const image_data& data)
{
    image_data D = data;
    switch(D.type)
    {
        case FIT_BITMAP:
            D.type = GL_UNSIGNED_BYTE;
            switch(D.format)
            {
                case 8:
                D.format = GL_RED;
                D.internal_format = GL_R8;
                break;

                case 16:
                D.format = GL_RG;
                D.internal_format = GL_RG8;
                break;

                case 24:
                D.format = GL_BGR;
                D.internal_format = GL_RGB8;
                break;

                case 32:
                D.format = GL_BGRA;
                D.internal_format = GL_RGBA8;
                break;
            }
            break;
        case FIT_RGBF:
            D.type = GL_FLOAT;
            D.format = GL_RGB;
            D.internal_format = GL_RGB32F;
            break;
        case FIT_RGBAF:
            D.type = GL_FLOAT;
            D.format = GL_RGBA;
            D.internal_format = GL_RGBA32F;
            break;
        case FIT_FLOAT:
            D.type = GL_FLOAT;
            D.format = GL_RED;
            D.internal_format = GL_R32F;
            break;
    }
    return D;
}

void Material::set_texture(texture& T, image_data& D, int slot)
{
    T.setWidth(D.width);
    T.setHeight(D.height);
    T.setSlot(slot);
    T.setTarget(GL_TEXTURE_2D);
    T.generate();
    T.setType(D.type);
    T.setFormat(D.format, D.internal_format);
    T.bind();
    T.genTexImage(D.data);
    T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    T.setTexParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    T.setTexParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    T.unbind();
    delete[] D.data;
}