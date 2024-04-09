#include <iostream>
#include <GL/glew.h>
#include "framebuffer.hpp"

void framebuffer::gen_FBO()
{
        glGenFramebuffers(1, &m.FBO);
}
framebuffer::~framebuffer()
{
        if (m.FBO)
                glDeleteFramebuffers(1, &m.FBO);
        if (m.RBO)
                glDeleteRenderbuffers(1, &m.RBO);
}

void framebuffer::bind()
{
        glBindFramebuffer(GL_FRAMEBUFFER, m.FBO);
}

void framebuffer::unbind()
{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::resize(unsigned int w, unsigned int h)
{
        m.w = w;
        m.h = h;

        if (!m.FBO)
                return;
        
        glDeleteFramebuffers(1, &m.FBO);
        glGenFramebuffers(1, &m.FBO);

        glDeleteRenderbuffers(1, &m.RBO);
        for (auto &T : m.color_attachments){
                glDeleteTextures(1, &T.m.texture);
                T.generate();
                T.setWidth(w);
                T.setHeight(h);
                T.bind();
                T.genTexImage(nullptr);
                T.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                T.unbind();
        }
        if (m.depth_attachment){
                texture &T = m.depth_attachment;
                glDeleteTextures(1, &T.m.texture);
                T.generate();
                T.setWidth(w);
                T.setHeight(h);
                T.bind();
                T.genTexImage(nullptr);
                T.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                T.unbind();

        }
        setup();
}

bool framebuffer::getFramebufferStatus()
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
        return false;
}


void framebuffer::setup()
{
        std::vector<unsigned int> drawBuffers;
        drawBuffers.reserve(m.color_attachments.size());

        bind();

        for(int i = 0; i < m.color_attachments.size(); i++) {
                texture& T = m.color_attachments[i];
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

void framebuffer::bindTextures()
{
        for(auto &T : m.color_attachments)
        {
                T.bind_unit();
        }
        if (m.depth_attachment)
                m.depth_attachment.bind_unit();
}

void framebuffer::create_RBO()
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
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, m.w, m.h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, m.RBO);
}

texture& framebuffer::gen_depth_attachment(const texture::texture_parameters& tp)
{
        m.depth_attachment = texture(tp);
        m.depth_attachment.generate();
        m.depth_attachment.bind();
        m.depth_attachment.genTexImage(nullptr);
        m.depth_attachment.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m.depth_attachment.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        m.depth_attachment.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m.depth_attachment.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m.depth_attachment.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        m.depth_attachment.unbind();
        return m.depth_attachment;
}


texture& framebuffer::gen_color_attachment(const texture::texture_parameters& tp)
{
        texture T(tp);
        T.generate();
        T.bind();
        T.genTexImage(nullptr);
        T.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        T.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        T.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        T.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        T.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        T.unbind();
        m.color_attachments.push_back(T);
        return m.color_attachments.back();
}
