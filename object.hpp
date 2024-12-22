#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "shader.hpp"

float CubeVertices[] = {
    // Position          // Normals         // Texture coordinations
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f
};
unsigned int CubeIndices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

class Cube {
public:
    Cube() : vao(0), vbo(0), ebo(0) {
        setupCube();
    }

    ~Cube() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    void setShader(Shader &shader) {
        shader = shader;
    }

    void render(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model,
                const glm::vec3& color, int texture1ID = -1, int texture2ID = -1, float textureMixFactor = 0.0f) {
        //shader.useShaderProgram();
        // Transformační matice
        /*glUniformMatrix4fv(glGetUniformLocation(shader.getShaderProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getShaderProgram(), "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getShaderProgram(), "model"), 1, GL_FALSE, &model[0][0]);

        // Barva
        glUniform3fv(glGetUniformLocation(shader.getShaderProgram(), "color"), 1, &color[0]);

        // Aktivace textur
        if (texture1ID >= 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1ID);
            glUniform1i(glGetUniformLocation(shader.getShaderProgram(), "texture1"), 0);
        }

        if (texture2ID >= 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2ID);
            glUniform1i(glGetUniformLocation(shader.getShaderProgram(), "texture2"), 1);
        }

        // Mixovací faktor pro textury
        glUniform1f(glGetUniformLocation(shader.getShaderProgram(), "textureMixFactor"), textureMixFactor);

        // Vykreslení krychle
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);*/

        // }

        shader.useShaderProgram();

        std::cerr << "shader.getShaderProgram(): " << shader.getShaderProgram() << std::endl;

        glUniformMatrix4fv(glGetUniformLocation(shader.getShaderProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getShaderProgram(), "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getShaderProgram(), "model"), 1, GL_FALSE, &model[0][0]);

        glUniform3fv(glGetUniformLocation(shader.getShaderProgram(), "color"), 1, &color[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1ID);
        glUniform1i(glGetUniformLocation(shader.getShaderProgram(), "textureSampler"), 0);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void setupCube() {
        GLuint vpos_location, vcol_location, tex_location;
        vpos_location = glGetAttribLocation(shader.getShaderProgram(), "aPos");
        vcol_location = glGetAttribLocation(shader.getShaderProgram(), "aNormal");
        tex_location  = glGetAttribLocation(shader.getShaderProgram(), "aTexCoords");


        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndices), CubeIndices, GL_STATIC_DRAW);

        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(vpos_location);

        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(vcol_location);

        glVertexAttribPointer(tex_location, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(tex_location);

    }

private:

    Shader shader;
    unsigned int vao, vbo, ebo;
};


#include <cmath>
#include <vector>

struct Vertex {
    float x, y, z;
    float nx, ny, nz; // Normály
    float u, v;       // UV souřadnice
};

std::vector<Vertex> generateSphere(float radius, int latitudeSegments, int longitudeSegments) {
    std::vector<Vertex> vertices;

    for (int lat = 0; lat <= latitudeSegments; ++lat) {
        float theta = lat * M_PI / latitudeSegments; // Úhel od pólu k pólu
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longitudeSegments; ++lon) {
            float phi = lon * 2.0f * M_PI / longitudeSegments; // Úhel kolem koule
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            // Souřadnice vrcholu
            float x = radius * cosPhi * sinTheta;
            float y = radius * cosTheta;
            float z = radius * sinPhi * sinTheta;

            // Normály
            float nx = cosPhi * sinTheta;
            float ny = cosTheta;
            float nz = sinPhi * sinTheta;

            // UV souřadnice
            float u = 1.0f - (lon / (float)longitudeSegments);
            float v = 1.0f - (lat / (float)latitudeSegments);

            vertices.push_back({x, y, z, nx, ny, nz, u, v});
        }
    }

    return vertices;
}



const char* shaderVertex =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "layout (location = 2) in vec2 aTexCoords;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "out vec2 TexCoords;\n"
    "\n"
    "void main()\n"
    "{\n"
    "FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "Normal = mat3(transpose(inverse(model))) * aNormal;\n"
    "TexCoords = aTexCoords;\n"
    "gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

const char* shaderFragment =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec3 Normal;\n"
    "in vec2 TexCoords;\n"
    "\n"
    "uniform vec3 color;\n"
    "uniform sampler2D textureSampler;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec4 texColor = texture(textureSampler, TexCoords);\n"
    "    vec3 norm = normalize(Normal);\n"
    "    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));\n"
    "    float diff = max(dot(norm, lightDir), 0.0);\n"
    "    vec3 mixedColor = mix(texColor.rgb, color, 0.5); //%50 color , 50 texture\n"
    "    vec3 lighting = diff * mixedColor;\n"
    "    FragColor = vec4(lighting, texColor.a);\n"
    "}\n";


