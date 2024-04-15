#include <iostream>
#include <cstring>
#include <GL/glew.h>
#include <SDL_assert.h>
#include "texture_load.hpp"

image_data load_image(const std::filesystem::path& P)
{
        fipImage image;
        if (!image.load(P.c_str(), 0))
        {
                std::cerr << "ERROR: couldn't load image at " << P << "\n";
                SDL_assert(false);
                return {};
        }
        image_data data;
        data.width = image.getWidth();
        data.height = image.getHeight();
        data.data_len = image.getHeight()*image.getScanWidth();
        data.data = new char[data.data_len];
        memcpy(data.data, image.accessPixels(), data.data_len);
        data.type = image.getImageType();
        data.format = image.getBitsPerPixel();
        return data;
}

image_data process_image_data(const image_data& data)
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
        case FIT_RGBA16:
            D.type =  GL_UNSIGNED_SHORT;
            D.format = GL_RGBA;
            D.internal_format = GL_RGBA16;
            break;
        case FIT_RGB16:
            D.type =  GL_UNSIGNED_SHORT;
            D.format = GL_RGB;
            D.internal_format = GL_RGB16UI;
            break;
        default:
            std::cerr << "Warning! unidentified format!" << std::endl;
            SDL_assert(false);
            break;
    }
    return D;
}