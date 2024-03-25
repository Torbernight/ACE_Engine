#pragma once

#include <utility>

class texture
{
        public:

        void generate();
        void bind();

        void bind_unit();

        void unbind();

        void setTarget(unsigned int target)
        {
                m.target = target;
        }

        void setSlot(unsigned int slot)
        {
                m.slot = slot;
        }

        void setWidth(unsigned int width){ m.width = width;}
        void setHeight(unsigned int height){ m.height = height;}
        void setDepth(unsigned int depth){ m.depth = depth;}
        void setType(unsigned int type){m.type = type;}
        void setFormat(unsigned int format, unsigned int internal_format)
        {
                m.format = format; 
                m.internal_format = internal_format;
        }

        void setTexParameter(unsigned int parameter, int value);
        void setTexParameter(unsigned int parameter, float value);
        void setTexParameter(unsigned int parameter, const float *value);
        void setTexParameter(unsigned int parameter, const int *value);

        void genTexImage(const void* pixels);

        texture(const texture& T):
        m(T.m)
        {
                const_cast<texture&>(T).m = {};
        }
        texture& operator=(const texture& T)
        {
                texture temp(T);
                return (*this = temp);
        }

        texture(texture&& T):
        m(std::move(T.m))
        {}

        texture(texture &T):
        m(T.m)
        {
                T.m = {};
        }

        texture& operator=(texture&& T)
        {
                texture temp(T);
                m = temp.m;
                temp.m = {};
                return *this;
        }
        texture& operator=(texture& T)
        {
                texture temp(T);
                m = temp.m;
                temp.m = {};
                return *this;         
        }
        texture()
        {
                m = {};
        }
        ~texture();

        private:
        struct members
        {
                unsigned int texture = 0;
                unsigned int target = 0;
                unsigned int slot = 0;
                unsigned int format = 0, internal_format = 0;
                unsigned int type = 0;
                unsigned int width, height, depth;
        } m;
};