#pragma once
#include "../camera.hpp"

class editor_camera : public camera
{
        public:

        void move(float dt) override;        

};