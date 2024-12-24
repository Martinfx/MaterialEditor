#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image.h"

class TextureManager {
public:
    struct Texture {
        unsigned int id;
        int width, height, channels;
        std::string path;
    };

    // Load a texture from file
    unsigned int loadTexture(const std::string& filepath) {
        if (textures_.count(filepath)) {
            return textures_[filepath].id;
        }

        Texture texture;
        texture.path = filepath;

        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);

        // Load the texture image
        unsigned char* data = stbi_load(filepath.c_str(), &texture.width, &texture.height, &texture.channels, 0);
        if (data) {
            GLenum format;
            if (texture.channels == 1) {
                format = GL_RED;
            }
            else if (texture.channels == 3) {
                format = GL_RGB;
            }
            else if (texture.channels == 4) {
                format = GL_RGBA;
            };

            glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            textures_[filepath] = texture;

            std::cout << "Texture loaded: " << filepath << std::endl;
        } else {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            glDeleteTextures(1, &texture.id);
            return 0;
        }

        return texture.id;
    }

    // Retrieve a texture by path
    const Texture* getTexture(const std::string& filepath) const {
        auto it = textures_.find(filepath);
        if (it != textures_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Save texture information
    void saveTextures(const std::string& savePath) const {
        std::ofstream file(savePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for saving textures: " << savePath << std::endl;
            return;
        }

        for (const auto& [path, texture] : textures_) {
            file << path << std::endl;
        }

        file.close();
        std::cout << "Textures saved to: " << savePath << std::endl;
    }

    // Load texture information
    void loadTextures(const std::string& loadPath) {
        std::ifstream file(loadPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for loading textures: " << loadPath << std::endl;
            return;
        }

        std::string path;
        while (std::getline(file, path)) {
            loadTexture(path);
        }

        file.close();
        std::cout << "Textures loaded from: " << loadPath << std::endl;
    }

    // Cleanup textures
    void cleanup() {
        for (const auto& [_, texture] : textures_) {
            glDeleteTextures(1, &texture.id);
        }
        textures_.clear();
    }

private:
    std::unordered_map<std::string, Texture> textures_;
};


