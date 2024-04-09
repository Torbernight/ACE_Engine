#include <iostream>
#include <cstring>
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