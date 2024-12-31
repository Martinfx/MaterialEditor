#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>
#include "debug.hpp"

class FrameBuffer
{
public:
    FrameBuffer();
    FrameBuffer(float width, float height);
    ~FrameBuffer();

    void InitFrameBuffer(float width, float height);
    unsigned int getFrameTexture();
    unsigned int getFrameBuffer();
    void RescaleFrameBuffer(float width, float height);
    void Bind() const;
    void Unbind() const;
    int GetWidth() { return width; }
    int GetHeight() { return height; }

private:
    unsigned int framebuffer;
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
    glDeleteTextures(1, &texColorBuffer);
    glDeleteRenderbuffers(1, &rbo);
}

void FrameBuffer::InitFrameBuffer(float width, float height)
{
    this->width = width;
    this->height = height;

    glGenFramebuffers(1, &framebuffer);
    glCheckError();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
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
    glCheckError();

}

unsigned int FrameBuffer::getFrameTexture()
{
    return texColorBuffer;
}

unsigned int FrameBuffer::getFrameBuffer()
{
    return framebuffer;
}

void FrameBuffer::RescaleFrameBuffer(float width, float height)
{
    this->width = width;
    this->height = height;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);


    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

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
