#pragma once
#include "../camera.hpp"

class editor_camera : public Camera
{
        public:

        void move(float dt) override;        

};