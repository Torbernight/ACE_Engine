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

imageArray_data load_image_array(const std::filesystem::path& P)
{
    fipMultiPage image;
        
    if (!image.open(P.c_str(), false, false, 0))
    {
        std::cerr << "ERROR: couldn't load image at " << P << "\n";
        SDL_assert(false);
        return {};
    }
    imageArray_data data;
    data.layers = image.getPageCount();
    data.data = new image_data[data.layers];

    for (int n = 0; n < data.layers; n++){
        fipImage layer;
        layer = image.lockPage(n);
        if(!layer.isValid()){
            std::cerr << "ERROR loading layer " << n << "\n";
            return {};
        }
        image_data &D = data.data[n];
        D.width = layer.getWidth();
        D.height = layer.getHeight();
        D.data_len = layer.getHeight()*layer.getScanWidth();
        D.data = new char[D.data_len];
        memcpy(D.data, layer.accessPixels(), D.data_len);
        D.type = layer.getImageType();
        D.format = layer.getBitsPerPixel();
        D = process_image_data(D);
        image.unlockPage(layer, false);
    }

    return data;
}
