#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <iostream>
#include "debug.hpp"

class FrameBuffer
{
public:
    FrameBuffer();
    FrameBuffer(float width, float height);
    ~FrameBuffer();

    void InitFrameBuffer(float width, float height);
    void AddTextureAttachment(GLenum format, GLenum attachment);
    unsigned int getFrameTexture(unsigned int index = 0);
    void SetViewerTextureIndex(unsigned int index);
    void RescaleFrameBuffer(float width, float height);
    void Bind() const;
    void Unbind() const;
    int GetWidth() { return width; }
    int GetHeight() { return height; }

private:
    unsigned int framebuffer;
    std::vector<GLuint> textures;
    unsigned int rbo;
    int width;
    int height;
    unsigned int viewerTextureIndex; // Index of the viewer texture
    unsigned int texColorBuffer = -1;
};

FrameBuffer::FrameBuffer()
    : framebuffer(0), rbo(0), width(0), height(0), viewerTextureIndex(0) {}

FrameBuffer::FrameBuffer(float width, float height)
    : framebuffer(0), rbo(0), viewerTextureIndex(0)
{
    InitFrameBuffer(width, height);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(textures.size(), textures.data());
    glDeleteRenderbuffers(1, &rbo);
}

void FrameBuffer::InitFrameBuffer(float width, float height)
{
    this->width = width;
    this->height = height;

    glGenFramebuffers(1, &framebuffer);
    glCheckError();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    //AddTextureAttachment(GL_RGB, GL_COLOR_ATTACHMENT0);
    //AddTextureAttachment(GL_RGB, GL_COLOR_ATTACHMENT1);
    glCheckError();

    glGenTextures(1, &texColorBuffer);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glCheckError();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glCheckError();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
    glCheckError();

    glGenRenderbuffers(1, &rbo);
    glCheckError();
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glCheckError();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glCheckError();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void FrameBuffer::AddTextureAttachment(GLenum format, GLenum attachment)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (attachment == GL_DEPTH_ATTACHMENT) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                     GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);

    textures.push_back(texture);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete after adding attachment!" << std::endl;
    }
}

unsigned int FrameBuffer::getFrameTexture(unsigned int index)
{
   /* if (index >= textures.size()) {
        std::cerr << "ERROR::FRAMEBUFFER:: Invalid texture index requested!" << std::endl;
        return 0;
    }
    return textures.at(index);*/
    return texColorBuffer;
}

void FrameBuffer::SetViewerTextureIndex(unsigned int index)
{
    if (index < textures.size()) {
        viewerTextureIndex = index;
    } else {
        std::cerr << "ERROR::FRAMEBUFFER:: Invalid viewer texture index!" << std::endl;
    }
}

void FrameBuffer::RescaleFrameBuffer(float width, float height)
{
    this->width = width;
    this->height = height;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    for (unsigned int texture : textures) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glCheckError();
}

void FrameBuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCheckError();
}
