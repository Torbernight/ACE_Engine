#pragma once

#include <filesystem>
#include "../texture.hpp"
#include "../texture_load.hpp"

typedef std::filesystem::path path;

class Material
{
    public:
    texture Albedo;
    //texture Normal;

    image_data process_image_data(const image_data& data);

    
    void load(const path& albedo)
    {
        image_data alb_data = process_image_data(load_image(albedo));
        //image_data nor_data = process_image_data(load_image(normal));
        set_texture(Albedo, alb_data, last_slot++);
        //set_texture(Normal, nor_data);
    }

    void bind()
    {
        Albedo.bind_unit();
        Albedo.bind();
    }

    private:
    static void set_texture(texture& T, image_data& D, int slot);
    
    inline static int last_slot = 0;

};
