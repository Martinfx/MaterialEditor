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
float CubeIndices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
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
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

void generateSphere(float radius, int sectorCount, int stackCount) {
    vertices.clear();
    indices.clear();

    float x, y, z, xy;                          // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                 // vertex texCoord

    float sectorStep = 2.0f * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    // Generování vertexů
    for(int i = 0; i <= stackCount; ++i) {
        stackAngle = glm::pi<float>() / 2 - i * stackStep;        // od -pi/2 do pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        for(int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;           // od 0 do 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::vec3(nx, ny, nz);
            vertex.texCoords = glm::vec2(s, t);
            vertices.push_back(vertex);
        }
    }

    // Generování indexů
    for(int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);     // začátek aktuálního stacku
        int k2 = k1 + sectorCount + 1;      // začátek dalšího stacku

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 trojúhelníky na čtverec
            if(i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if(i != (stackCount-1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
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


