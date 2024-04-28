#pragma once

#include <FreeImagePlus.h>
#include <filesystem>

struct image_data
{
        unsigned int width, height;
        char* data;
        size_t data_len;
        unsigned int type;
        unsigned int format, internal_format;
};

image_data load_image(const std::filesystem::path& P);
image_data process_image_data(const image_data& data);

struct imageArray_data
{
        size_t layers;
        image_data *data;
};
imageArray_data load_image_array(const std::filesystem::path& P);