#include <cstring>
#include <FreeImagePlus.h>
#include "texture_load.hpp"

image_data load_image(const std::filesystem::path& P)
{
        fipImage image;
        image.load(P.c_str(), 0);
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