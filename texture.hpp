#pragma once

#include <iostream>
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image.h"
#include "debug.hpp"

class TextureManager
{
public:

    TextureManager() {}
    ~TextureManager() {}

    unsigned int loadTexture(const std::string &name)
    {
        glGenTextures(1, &m_id);
        int width, height, nrChannels;
        unsigned char *data = stbi_load(name.c_str(), &width, &height, &nrChannels, 0);
        if(data)
        {
            GLenum format = 0;
            if (nrChannels == 1)
            {
                format = GL_RED;
            }
            else if (nrChannels == 3)
            {
                format = GL_RGB;
            }
            else if (nrChannels == 4)
            {
                format = GL_RGBA;
            }

            glBindTexture(GL_TEXTURE_2D, m_id);
             glCheckError();
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
              glCheckError();
             glGenerateMipmap(GL_TEXTURE_2D);
             glCheckError();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glCheckError();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glCheckError();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glCheckError();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glCheckError();
            std::cerr << "Load texture: " << name << std::endl;
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Failed to load texture: " << name << std::endl;
            stbi_image_free(data);
        }

        return m_id;
    }

    unsigned int loadCubeTexture(std::vector<std::string> &faces)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if(data)
            {
                GLenum format = 0;
                if (nrChannels == 1)
                {
                    format = GL_RED;
                }
                else if (nrChannels == 3)
                {
                    format = GL_RGB;
                }
                else if (nrChannels == 4)
                {
                    format = GL_RGBA;
                }

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }


private:

    unsigned int m_id;
};

