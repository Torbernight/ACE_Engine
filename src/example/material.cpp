#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include "debug/debug.hpp"
#include "material.hpp"

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
    T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    T.setTexParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    T.setTexParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    T.generate_mipmap();
    T.unbind();
    delete[] D.data;
}