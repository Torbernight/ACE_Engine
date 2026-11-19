#pragma once

#include <FreeImagePlus.h>
#include <filesystem>

namespace Renderer 
{

struct ImageData
{
        unsigned int width, height;
        char* data;
        size_t data_len;
        unsigned int type;
        unsigned int format, internal_format;
};

ImageData load_image(const std::filesystem::path& P);
ImageData process_image_data(const ImageData& data);

struct ImageArrayData
{
        size_t layers;
        ImageData *data;
};
ImageArrayData load_image_array(const std::filesystem::path& P);

}