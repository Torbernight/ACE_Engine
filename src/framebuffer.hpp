#pragma once
#include <vector>
#include "texture.hpp"

class framebuffer
{
        public:

        ~framebuffer();

        void gen_FBO();

        void setup();

        int getMSAA() const {return m.MSAA;}

        unsigned int getWidth() const {return m.w;}
        unsigned int getHeight() const {return m.h;}

        void resize(unsigned int w, unsigned int h);
        void reset(int MSAA = 0);

        texture& gen_color_attachment(const texture::texture_parameters& tp, int MSAA = 0);
        texture& gen_depth_attachment(const texture::texture_parameters& tp, int MSAA = 0);

        void bind();
        static void unbind();

        void bindTextures();

        private:

        void create_RBO();

        bool getFramebufferStatus();

        struct members{
                unsigned int FBO = 0;
                unsigned int RBO = 0;
                std::vector<texture> color_attachments;
                texture depth_attachment;
                unsigned int w = 0, h = 0;
                int MSAA = 0;
        } m;
};