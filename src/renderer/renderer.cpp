#include <SDL_assert.h>
#include "renderer.hpp"

namespace Renderer 
{

LightID State::add_light(const Light& light)
{
    LightID result = Lights.size();
    Lights.push_back(light);
    return result;
}
Light& State::get_light(LightID ID)
{
    SDL_assert(ID < Lights.size());
    return Lights[ID];
}
const Light& State::get_light(LightID ID) const
{
    SDL_assert(ID < Lights.size());
    return Lights[ID];
}

CameraID State::add_camera(const Camera& camera)
{
    CameraID result = Cameras.size();
    Cameras.push_back(camera);
    return result;
}
Camera& State::get_camera(CameraID ID)
{
    SDL_assert(ID < Cameras.size());
    return Cameras[ID];
}
const Camera& State::get_camera(CameraID ID) const
{
    SDL_assert(ID < Cameras.size());
    return Cameras[ID];
}

MaterialID State::add_material(const Material& material)
{
    MaterialID result = Materials.size();
    Materials.push_back(material);
    return result;
}
Material& State::get_material(MaterialID ID)
{
    SDL_assert(ID < Materials.size());
    return Materials[ID];
}
const Material& State::get_material(MaterialID ID) const
{
    SDL_assert(ID < Materials.size());
    return Materials[ID];
}

}