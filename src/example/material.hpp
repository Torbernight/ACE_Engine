#pragma once

#include <filesystem>
#include <utility>
#include "../texture.hpp"
#include "../texture_load.hpp"

typedef std::filesystem::path path;

class Material
{
    public:

    void load(const path& albedo, const path& normal, const path& roughness)
    {
        image_data alb_data = process_image_data(load_image(albedo));
        image_data nor_data = process_image_data(load_image(normal));
        image_data roug_data = process_image_data(load_image(roughness));
        set_texture(m.Albedo, alb_data, last_slot++);
        set_texture(m.Normal, nor_data, last_slot++);
        set_texture(m.Roughness, roug_data, last_slot++);
    }

    void loadAlbedo(const path& albedo)
    {
        image_data alb_data = process_image_data(load_image(albedo));
        set_texture(m.Albedo, alb_data, last_slot++);
    }

    void loadNormal(const path& normal)
    {
        image_data nor_data = process_image_data(load_image(normal));
        set_texture(m.Normal, nor_data, last_slot++);
    }

    void loadRoughness(const path& roughness)
    {
        image_data roug_data = process_image_data(load_image(roughness));
        set_texture(m.Roughness, roug_data, last_slot++);
    }

    void bind()
    {
        m.Albedo.bind_unit();
        m.Normal.bind_unit();
    }

    texture& get_albedo()
    {
        return m.Albedo;
    }
    const texture& get_albedo() const
    {
        return m.Albedo;
    }
    texture& get_normal()
    {
        return m.Normal;
    }
    const texture& get_normal() const
    {
        return m.Normal;
    }
    texture& get_roughness()
    {
        return m.Roughness;
    }
    const texture& get_roughness() const
    {
        return m.Roughness;
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
        texture Normal;
        texture Roughness;
    } m;

    static image_data process_image_data(const image_data& data);
    static void set_texture(texture& T, image_data& D, int slot);
    static int last_slot;

};
