#include <cassert>
#include <iostream>
#include <glad/gl.h>
#include "framebuffer.hpp"

namespace Renderer {

void Framebuffer::gen_FBO()
{
        glGenFramebuffers(1, &m.FBO);
}
Framebuffer::~Framebuffer()
{
        if (m.FBO)
                glDeleteFramebuffers(1, &m.FBO);
        if (m.RBO)
                glDeleteRenderbuffers(1, &m.RBO);
}

void Framebuffer::bind()
{
        glBindFramebuffer(GL_FRAMEBUFFER, m.FBO);
}

void Framebuffer::unbind()
{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::reset(int MSAA)
{
        this->m.MSAA = MSAA;

        if (!m.FBO)
                return;
        
        glDeleteFramebuffers(1, &m.FBO);
        glGenFramebuffers(1, &m.FBO);
        glDeleteRenderbuffers(1, &m.RBO);

        for (auto &T : m.color_attachments){
                glDeleteTextures(1, &T.m.texture);
                if (m.MSAA != 0){
                        T.setTarget(GL_TEXTURE_2D_MULTISAMPLE);
                } else{
                        T.setTarget(GL_TEXTURE_2D);
                }
                T.generate();
                T.setWidth(m.w);
                T.setHeight(m.h);
                T.bind();
                if (m.MSAA != 0){
                        T.genTexImageMS(m.MSAA);               
                } else {
                        T.genTexImage(nullptr);
                        T.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        T.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                        T.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                T.unbind();
        }
        if (m.depth_attachment){
                Texture &T = m.depth_attachment;
                glDeleteTextures(1, &T.m.texture);
                if (m.MSAA){
                        T.setTarget(GL_TEXTURE_2D_MULTISAMPLE);
                } else{
                        T.setTarget(GL_TEXTURE_2D);
                }
                T.generate();
                T.setWidth(m.w);
                T.setHeight(m.h);
                T.bind();
                if (m.MSAA){
                        T.genTexImageMS(m.MSAA);               
                } else {
                        T.genTexImage(nullptr);
                        T.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        T.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                        T.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                T.unbind();
        }
        setup();
}


void Framebuffer::resize(unsigned int w, unsigned int h)
{
        m.w = w;
        m.h = h;
        reset(m.MSAA);
}
bool Framebuffer::getFramebufferStatus()
{
        switch(glCheckFramebufferStatus(GL_FRAMEBUFFER))
        {
                case GL_FRAMEBUFFER_COMPLETE:
                        return true;
                case GL_FRAMEBUFFER_UNDEFINED:
                        std::cerr << "Error: Framebuffer incomplete!\nstatus: GL_FRAMEBUFFER_UNDEFINED\n";
                        break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                        std::cerr << "Error: Framebuffer incomplete!\nstatus: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n";
                        break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                        std::cerr << "Error: Framebuffer incomplete!\nstatus: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n";
                        break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                        std::cerr << "Error: Framebuffer incomplete!\nstatus: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n";
                        break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                        std::cerr << "Error: Framebuffer incomplete!\nstatus: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n";
                        break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                        std::cerr << "Error: Framebuffer incomplete! status GL_FRAMEBUFFER_UNSUPPORTED\n";
                        break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                        std::cerr << "Error: Framebuffer incomplete! status GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n";
                        break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                        std::cerr << "Error: Framebuffer incomplete! status GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n";
                        break;
        }
        assert(false);
        return false;
}


void Framebuffer::setup()
{
        std::vector<unsigned int> drawBuffers;
        drawBuffers.clear();
        drawBuffers.reserve(m.color_attachments.size());

        bind();

        for(int i = 0; i < m.color_attachments.size(); i++) {
                Texture& T = m.color_attachments[i];
                unsigned int attachment = GL_COLOR_ATTACHMENT0+i;
                unsigned int target = T.get_Target();
                unsigned int tex = T.get_TexObject();
                switch(T.get_target_dims()) {
                        case 1:
                                glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, target, tex, 0);
                                break;
                        case 2:
                                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, tex, 0);
                                break;
                        case 3:
                                glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, target, tex, 0, 0); //to do: set layer?
                                break;
                }
                drawBuffers.push_back(attachment);
        }

        if (m.depth_attachment){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m.depth_attachment.get_Target(), m.depth_attachment.get_TexObject(), 0);
        }
        
        create_RBO();

        glDrawBuffers(drawBuffers.size(), drawBuffers.data());
        getFramebufferStatus();
        unbind();
}

void Framebuffer::bindTextures()
{
        for(auto &T : m.color_attachments)
        {
                T.bind_unit();
        }
        if (m.depth_attachment)
                m.depth_attachment.bind_unit();
}

void Framebuffer::create_RBO()
{
        unsigned int internalFormat = 0;
        unsigned int attachment = 0;

        if(!m.depth_attachment){
                internalFormat = GL_DEPTH24_STENCIL8;
                attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        } else {
                return;
        }
        glGenRenderbuffers(1, &m.RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m.RBO);
        if (m.MSAA == 0){
                glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, m.w, m.h);
        } else {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, m.MSAA, internalFormat, m.w, m.h);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, m.RBO);
}

Texture& Framebuffer::gen_depth_attachment(const Texture::TextureParameters& tp, int MSAA)
{
        this->m.MSAA = MSAA;
        m.depth_attachment = Texture(tp);
        m.depth_attachment.generate();
        m.depth_attachment.bind();
        if (MSAA){
                m.depth_attachment.genTexImageMS(MSAA);               
        } else {
                m.depth_attachment.genTexImage(nullptr);
                m.depth_attachment.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                m.depth_attachment.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                m.depth_attachment.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                m.depth_attachment.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                m.depth_attachment.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        m.depth_attachment.unbind();
        return m.depth_attachment;
}


Texture& Framebuffer::gen_color_attachment(const Texture::TextureParameters& tp, int MSAA)
{
        this->m.MSAA = MSAA;
        Texture T(tp);
        T.generate();
        T.bind();
        if (T.get_Target() == GL_TEXTURE_2D_MULTISAMPLE){
                T.genTexImageMS(MSAA);               
        } else {
                T.genTexImage(nullptr);
                T.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        T.unbind();
        m.color_attachments.push_back(T);
        return m.color_attachments.back();
}

}