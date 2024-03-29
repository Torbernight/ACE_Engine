#pragma once

#include <filesystem>
#include <utility>
#include "../texture.hpp"
#include "../texture_load.hpp"

typedef std::filesystem::path path;

class Material
{
    public:

    void load(const path& albedo)
    {
        image_data alb_data = process_image_data(load_image(albedo));
        //image_data nor_data = process_image_data(load_image(normal));
        set_texture(m.Albedo, alb_data, last_slot);
        last_slot += 1;
        //set_texture(Normal, nor_data);
    }

    void bind()
    {
        m.Albedo.bind_unit();
    }

    unsigned int get_albedo_unit() const 
    {
        return m.Albedo.m.slot;
    }

    Material(const Material& M) :
    m(M.m)
    {
        const_cast<Material&>(M).m = {};
    }
    Material& operator=(const Material& M)
    {
        Material temp(M);
        return (*this = temp);
    }

    Material(Material& M) :
    m(M.m)
    {
        M.m = {};
    }
    Material(Material&& M) :
    m(std::move(M.m))
    {
    }
    Material& operator=(Material& M)
    {
        m = M.m;
        M.m = {};        
        return *this;
    }
    Material& operator=(Material&& M)
    {
        Material temp(M);
        return (*this = temp);
    }

    Material()
    {
        m = {};
    }

    private:

    struct members
    {
        texture Albedo;
        //texture Normal;
    } m;

    static image_data process_image_data(const image_data& data);
    static void set_texture(texture& T, image_data& D, int slot);
    static int last_slot;

};
