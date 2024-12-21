#pragma once

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

#include <vector>
#include <cmath>

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


