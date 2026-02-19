#pragma once
#include <vector>
#include "texture.hpp"

class Framebuffer
{
        public:

        ~Framebuffer();

        void gen_FBO();

        void setup();

        int getMSAA() const {return m.MSAA;}

        unsigned int getWidth() const {return m.w;}
        unsigned int getHeight() const {return m.h;}

        void resize(unsigned int w, unsigned int h);
        void reset(int MSAA = 0);

        Texture& gen_color_attachment(const Texture::TextureParameters& tp, int MSAA = 0);
        Texture& gen_depth_attachment(const Texture::TextureParameters& tp, int MSAA = 0);

        void bind();
        static void unbind();

        void bindTextures();

        private:

        void create_RBO();

        bool getFramebufferStatus();

        struct members{
                unsigned int FBO = 0;
                unsigned int RBO = 0;
                std::vector<Texture> color_attachments;
                Texture depth_attachment;
                unsigned int w = 0, h = 0;
                int MSAA = 0;
        } m;
};