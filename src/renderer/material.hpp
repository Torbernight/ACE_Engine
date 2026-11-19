#pragma once

#include <array>
#include <filesystem>
#include <utility>
#include "texture.hpp"
#include "texture_load.hpp"

typedef std::filesystem::path path;


namespace Renderer 
{
class Material
{
    public:

    enum Texture_Index
    {
        Albedo,
        Normal,
        Roughness,
        Emissive
    };

    Texture& load(const path& Path, Texture_Index tex)
    {
        ImageData data = process_image_data(load_image(Path));
        set_texture(m.textures[tex], data, last_slot++);
        return m.textures[tex];
    }

    void bind()
    {
        for(auto& T: m.textures){
            if (T)
                T.bind_unit();
        }
    }

    Texture& get(Texture_Index T)
    {
        return m.textures[T];
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

    inline static constexpr unsigned nTextures = 4;

    struct members
    {
        std::array<Texture, nTextures> textures;
    } m;

    static void set_texture(Texture& T, ImageData& D, int slot);
    static inline int last_slot = 0;

};

}