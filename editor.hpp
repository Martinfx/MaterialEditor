#pragma once

#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <imnodes.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "3rdparty/tinyfiledialogs/include/tinyfiledialogs/tinyfiledialogs.h"

#include "node.hpp"
#include "graph.hpp"
#include "object.hpp"
#include "shader.hpp"
#include "framebuffer.hpp"
#include "texture.hpp"
#include "debug.hpp"

template<class T>
T clamp(T x, T a, T b)
{
    return std::min(b, std::max(x, a));
}

static float current_time_seconds = 0.f;
static bool  emulate_three_button_mouse = false;

class NodeEditor
{
public:
    NodeEditor()
        : graph_(), nodes_(), root_node_id_(-1),
        minimap_location_(ImNodesMiniMapLocation_BottomRight)
    {

        setupSphere(1.0f, 16, 16);
        frameBuffer.InitFrameBuffer(800,600);
        glCheckError();
        //frameBuffer.AddTextureAttachment(GL_RGB, GL_COLOR_ATTACHMENT0);
        //glCheckError();
        //frameBuffer.AddTextureAttachment(GL_RGB, GL_COLOR_ATTACHMENT1);
        //glCheckError();
        //frameBuffer.SetViewerTextureIndex(1);
        frameBUfferSphere.InitFrameBuffer(800, 600);
        auto shaderCode = ShaderCodeGenerator::generateShaderCode(nodes_, graph_);
        mainShader.loadShaderFromString(shaderCode.vertexCode.c_str(), TypeShader::VERTEX_SHADER);
        mainShader.loadShaderFromString(shaderCode.fragmentCode.c_str(), TypeShader::FRAGMENT_SHADER);
        mainShader.createShaderProgram();

        // first viewer is cube.
        frameBuffer.Bind();
        render_to_framebuffer_cube(glm::vec3(1.0f, 0.5f, 0.31f));
        frameBuffer.Unbind();
    }

    ~NodeEditor()  {
        //textureManager.cleanup();
    }

private:
    unsigned int fbo;      // Framebuffer Object
    unsigned int texture = 0;
    unsigned int rbo, ebo;
    unsigned int cubeVAO, cubeVBO = 0;
    unsigned int sphereVAO, sphereVBO, sphereEBO = 0;
    Shader mainShader;
    FrameBuffer frameBuffer;
    FrameBuffer frameBUfferSphere;
    TextureManager textureManager;

    int m_latitudeSegments = 0;
    int m_longitudeSegments = 0;

    float rotationY = 0;
    float rotationX = 0;

    // blend
    unsigned int textureId, textureId2 = -1;
    float mixFactor = 0.7f;
    // adjust
    float brightness, contrast, saturation = 0.0f;
    // light
    glm::vec3 lightposition = glm::vec3(10.f,5.f,5.f);
    glm::vec3 lightcolor = glm::vec3(0.f,0.f,0.f);
    float lightintensity = 3.f;
    // material
    float materialambient = 0.f;
    float materialdiffuse = 0.f;
    float materialspecular = 0.f;
    float materialshininess = 0.f;

private:
    enum class UiNodeType
    {
        add,
        multiply,
        output,
        sine,
        time,
        power,
        cubeviewport,
        sphereviewport,
        texture,
        blend,
        colorAdjust,
        light,
        pointLight,
        directionalLight,
        spotLight,
        lightingModel,     // (Phong, Blinn-Phong)
        material
    };

    struct UiNode
    {
        UiNodeType type;
        // The identifying id of the ui node. For add, multiply, sine, and time
        // this is the "operation" node id. The additional input nodes are
        // stored in the structs.
        int id;

        union
        {
            struct
            {
                int lhs, rhs;
            } add;

            struct
            {
                int lhs, rhs;
            } multiply;

            struct
            {
                int lhs, rhs;
            } power;

            struct
            {
                int r, g, b;
            } output;

            struct
            {
                int input;
            } sine;

            struct
            {
                int input;
            } cubeviewport;

            struct
            {
                int input;
            } sphereviewport;

            struct
            {
                int id;
                char* path;
            }texture;

            struct
            {
                unsigned int id;
                unsigned int id2;
                char* path;
                char* path2;
                float mixFactor;
            }blend;

            struct
            {
                float brightness;
                float contrast;
                float saturation;
            }colorAdjust;

            struct {
                glm::vec3 position;
                glm::vec3 color;
                float intensity;
            } light;

            struct {
                glm::vec3 position;
                glm::vec3 color;
                float intensity;
                float radius;
                float attenuation;
            } pointLight;

            struct {
                glm::vec3 direction;
                glm::vec3 color;
                float intensity;
            } directionalLight;

            struct {
                glm::vec3 position;
                glm::vec3 direction;
                glm::vec3 color;
                float intensity;
                float cutOff;
                float outerCutOff;
            } spotLight;

            struct {
                float ambient;
                float diffuse;
                float specular;
                float shininess;
            } material;

        } ui;
    };

    class ShaderCodeGenerator {
    public:
        struct ShaderOutput {
            std::string vertexCode;
            std::string fragmentCode;
            std::vector<std::string> macros;    // Added macros for conditional inclusion
        };

        static ShaderOutput generateShaderCode(const std::vector<UiNode>& nodes, const Graph<Node>& graph) {
            ShaderOutput output;

            // Generate vertex shader
            output.vertexCode = generateVertexShader();

            // Generate fragment shader
            std::stringstream ss;

            // Shader Header
            ss << "#version 330 core\n\n";
            ss << "in vec2 TexCoords;\n";
            ss << "in vec3 Normal;\n";
            ss << "in vec3 FragPos;\n";
            ss << "out vec4 FragColor;\n\n";

            // Add macros for active nodes
            addActiveMacros(output.macros, nodes);

            // Include macros at the start of the shader
            for (const auto& macro : output.macros) {
                ss << "#define " << macro << "\n";
            }
            ss << "\n";

            // Uniform declarations
            ss << "uniform sampler2D texture1;\n";
            ss << "uniform sampler2D texture2;\n";
            ss << "uniform float mixFactor;\n";
            ss << "uniform float brightness;\n";
            ss << "uniform float contrast;\n";
            ss << "uniform float saturation;\n\n";
            ss << "uniform vec3 lightPos;\n";
            ss << "uniform vec3 lightColor;\n";
            ss << "uniform float lightIntensity;\n";
            ss << "uniform vec3 viewPos;\n\n";


            // Helper functions
            generateHelperFunctions(ss);

            // Main Function
            ss << "void main() {\n";
            ss << "    vec3 result = vec3(1.0);\n\n";

            // Topological Sort and Generate Node Code
            auto sortedNodes = topologicalSort(nodes, graph);
            for (int nodeId : sortedNodes) {
                const UiNode& node = findNode(nodes, nodeId);
                generateNodeCodeWithMacros(ss, node, graph);
            }

            // Final Output
            ss << "    FragColor = vec4(result, 1.0);\n";
            ss << "}\n";

            output.fragmentCode = ss.str();
            return output;
        }

    private:
        static std::string generateVertexShader() {
            return R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoords = aTexCoords;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
        )";
        }

        static void generateHelperFunctions(std::stringstream& ss) {
            ss << R"(
        vec3 adjustColor(vec3 color, float brightness, float contrast, float saturation) {
            color += brightness;
            color = (color - 0.5) * contrast + 0.5;
            float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
            color = mix(vec3(luminance), color, saturation);
            return clamp(color, 0.0, 1.0);
        }

        vec4 blendTextures(vec4 tex1, vec4 tex2, float factor) {
            return mix(tex1, tex2, factor);
        }
        )";
            ss << R"(
vec3 calculateLighting(vec3 normal, vec3 fragPos, vec3 viewDir, vec3 lightPos, vec3 lightColor, float lightIntensity) {
    // Ambient
    vec3 ambient = 0.1 * lightColor;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    return ambient + diffuse + specular;
}
)";
        }

        static void addActiveMacros(std::vector<std::string>& macros, const std::vector<UiNode>& nodes) {
            for (const auto& node : nodes) {
                switch (node.type) {
                case UiNodeType::texture:
                    macros.push_back("USE_TEXTURE_" + std::to_string(node.id));
                    break;
                case UiNodeType::blend:
                    macros.push_back("USE_BLEND_" + std::to_string(node.id));
                    break;
                case UiNodeType::colorAdjust:
                    macros.push_back("USE_COLOR_ADJUST_" + std::to_string(node.id));
                    break;
                case UiNodeType::light:
                    macros.push_back("USE_LIGHT_" + std::to_string(node.id));
                    break;
                default:
                    break;
                }
            }
        }

        static void generateNodeCodeWithMacros(std::stringstream& ss, const UiNode& node, const Graph<Node>& graph) {
            switch (node.type) {
            case UiNodeType::texture:
                ss << "#ifdef USE_TEXTURE_" << node.id << "\n";
                ss << "    vec4 tex" << node.id << " = texture(texture1, TexCoords);\n";
                ss << "    result *= tex" << node.id << ".rgb;\n";
                ss << "#endif\n";
                break;

            case UiNodeType::blend:
                ss << "#ifdef USE_BLEND_" << node.id << "\n";
                ss << "    vec4 blendResult" << node.id << " = blendTextures(\n";
                ss << "        texture(texture1, TexCoords),\n";
                ss << "        texture(texture2, TexCoords),\n";
                ss << "        mixFactor);\n";
                ss << "    result *= blendResult" << node.id << ".rgb;\n";
                ss << "#endif\n";
                break;

            case UiNodeType::colorAdjust:
                ss << "#ifdef USE_COLOR_ADJUST_" << node.id << "\n";
                ss << "    result = adjustColor(result,\n";
                ss << "        brightness,\n";
                ss << "        contrast,\n";
                ss << "        saturation);\n";
                ss << "#endif\n";
                break;
            case UiNodeType::light:
                ss << "#ifdef USE_LIGHT_" << node.id << "\n";
                ss << "    vec3 lightEffect = calculateLighting(Normal, FragPos, normalize(viewPos - FragPos),\n";
                ss << "                             lightPos, lightColor, lightIntensity);\n";
                ss << "    result *= lightEffect;\n";
                ss << "#endif\n";
                break;
            default:
                ss << "// Unsupported node type: "  << "\n";
                break;
            }
        }

        static std::vector<int> topologicalSort(const std::vector<UiNode>& nodes, const Graph<Node>& graph) {
            std::vector<int> result;
            std::unordered_set<int> visited;

            std::function<void(int)> dfs = [&](int nodeId) {
                if (visited.count(nodeId)) return;
                visited.insert(nodeId);

                for (const auto& edge : graph.edges()) {
                    if (edge.from == nodeId) {
                        dfs(edge.to);
                    }
                }

                result.push_back(nodeId);
            };

            for (const auto& node : nodes) {
                if (!visited.count(node.id)) {
                    dfs(node.id);
                }
            }

            std::reverse(result.begin(), result.end());
            return result;
        }

        static const UiNode& findNode(const std::vector<UiNode>& nodes, int id) {
            auto it = std::find_if(nodes.begin(), nodes.end(),
                                   [id](const UiNode& node) { return node.id == id; });
            if (it == nodes.end()) {
                std::cerr << ("Node with ID " + std::to_string(id) + " not found.");
            }
            return *it;
        }
    };



    Graph<Node>            graph_;
    std::vector<UiNode>    nodes_;
    int                    root_node_id_;
    ImNodesMiniMapLocation minimap_location_;
    bool                   showSphere;
    // check if is initialized cube vao,vbo,ebo buffer
    bool initialized =     false;

public:

    uint32_t getTicks() {
        static const auto startTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        return static_cast<uint32_t>(elapsed);
    }


    void renderCube(const glm::vec3& color, const glm::mat4& projection, const glm::mat4& view,
                    const glm::mat4& model) {
        if (!initialized) {

            glGenVertexArrays(1, &cubeVAO);
            glCheckError();
            glGenBuffers(1, &cubeVBO);
            glCheckError();
            glGenBuffers(1, &ebo);
            glCheckError();
            glBindVertexArray(cubeVAO);
            glCheckError();
            glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
            glCheckError();
            glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), &CubeVertices, GL_STATIC_DRAW);
            glCheckError();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glCheckError();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndices), &CubeIndices, GL_STATIC_DRAW);
            glCheckError();
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glCheckError();
            glEnableVertexAttribArray(0);
            glCheckError();
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glCheckError();
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glCheckError();
            glEnableVertexAttribArray(2);
            glCheckError();
            initialized = true;
        }

        mainShader.useShaderProgram();
        glCheckError();
        // Set transformation matrices
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glCheckError();
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "view"), 1, GL_FALSE, &view[0][0]);
        glCheckError();
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "model"), 1, GL_FALSE, &model[0][0]);
        glCheckError();
        // Base color
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), "baseColor"), 1, &color[0]);
        glCheckError();
        // Handle Blend Node


        //GLint viewPosLocation = glGetUniformLocation(mainShader.getShaderProgram(), "viewPos");
        //if (viewPosLocation == -1) {
        //    std::cerr << "Uniform 'viewPos' not found or unused in shader." << std::endl;
        //   }

        glm::vec3 cameraPosition = glm::vec3(3.0f, 3.0f, 3.0f);
        glUniform3f(glGetUniformLocation(mainShader.getShaderProgram(), "viewPos"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
        glCheckError();

        glActiveTexture(GL_TEXTURE0);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, textureId);
        std::cerr << "textureId : " << textureId << std::endl;
        glCheckError();
        mainShader.setUniformInt("texture1", 0);
        glCheckError();

        glActiveTexture(GL_TEXTURE1);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, textureId2);
        std::cerr << "textureId2 : " << textureId2 << std::endl;
        glCheckError();
        mainShader.setUniformInt("texture2", 1);
        glCheckError();
        GLint texture1Location = glGetUniformLocation(mainShader.getShaderProgram(), "texture1");
        if (texture1Location == -1) {
            std::cerr << "Uniform 'texture1' not found or unused in shader." << std::endl;
        }

        GLint texture2Location = glGetUniformLocation(mainShader.getShaderProgram(), "texture2");
        if (texture2Location == -1) {
            std::cerr << "Uniform 'texture2' not found or unused in shader." << std::endl;
        }

        glCheckError();
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "mixFactor"), mixFactor);
        std::cerr << "mixfactor : " << mixFactor << std::endl;
        glCheckError();
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "brightness"), brightness);
        glCheckError();
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "contrast"), contrast);
        glCheckError();
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "saturation"), saturation);
        glCheckError();

        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), "lightPos"), 1, &lightposition[0]);
        glCheckError();
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), "lightColor"), 1, &lightcolor[0]);
        glCheckError();
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "lightIntensity"), lightintensity);
        glCheckError();
        //glm::vec3 cameraPosition(30.0f, 30.0f, 30.0f); // Example camera position
        //glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), "viewPos"), 1, &cameraPosition[0]);

        //Material material = {0.2f, 0.5f, 0.3f, 32.0f}; // Example material
        //glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "material.ambient"), 0.2f);
        //glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "material.diffuse"),  0.5f);
        //glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "material.specular"), 0.3f);
        //glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "material.shininess"), 2.0f);

        //std::string lightUniformBase = "pointLights[" + std::to_string(1) + "].";
        //glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), (lightUniformBase + "position").c_str()), 1, &lightposition[0]);
        //glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), (lightUniformBase + "color").c_str()), 1, &lightcolor[0]);
        //glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), (lightUniformBase + "intensity").c_str()), lightintensity);
        /*
        std::string dirLightUniformBase = "directionalLight.";
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), (dirLightUniformBase + "direction").c_str()), 1, &node.ui.directionalLight.direction[0]);
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), (dirLightUniformBase + "color").c_str()), 1, &node.ui.directionalLight.color[0]);
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), (dirLightUniformBase + "intensity").c_str()), node.ui.directionalLight.intensity);

        std::string spotLightUniformBase = "spotLight.";
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), (spotLightUniformBase + "position").c_str()), 1, &node.ui.spotLight.position[0]);
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), (spotLightUniformBase + "direction").c_str()), 1, &node.ui.spotLight.direction[0]);
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), (spotLightUniformBase + "color").c_str()), 1, &node.ui.spotLight.color[0]);
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), (spotLightUniformBase + "intensity").c_str()), node.ui.spotLight.intensity);
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), (spotLightUniformBase + "cutOff").c_str()), node.ui.spotLight.cutOff);
        glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), (spotLightUniformBase + "outerCutOff").c_str()), node.ui.spotLight.outerCutOff);
*/
        //frameBuffer.Unbind();
        //mainShader.useShaderProgram();
        // Render the cube
        //glBindTexture(GL_TEXTURE_2D, frameBuffer.getFrameTexture());
        glBindVertexArray(cubeVAO);
        glCheckError();

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glCheckError();
        glBindVertexArray(0);
        glCheckError();
        //GLenum err;
        //while ((err = glGetError()) != GL_NO_ERROR) {
        //    std::cerr << "OpenGL error: " << err << std::endl;
        //}
    }

    void setupSphere(float radius, int latitudeSegments, int longitudeSegments) {
        generateSphere(radius, latitudeSegments, longitudeSegments);

        m_latitudeSegments = latitudeSegments;
        m_longitudeSegments = longitudeSegments;
        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);

        glBindVertexArray(sphereVAO);

        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);


    }

    void renderSphere(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model, const glm::vec3& color) {

        mainShader.useShaderProgram();

        glm::vec3 cameraPosition = glm::vec3(3.0f, 3.0f, 3.0f);
        glUniform3f(glGetUniformLocation(mainShader.getShaderProgram(), "viewPos"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
        glCheckError();

        glActiveTexture(GL_TEXTURE0);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, textureId);
        std::cerr << "textureId : " << textureId << std::endl;
        glCheckError();
        mainShader.setUniformInt("texture1", 0);
        glCheckError();

        glActiveTexture(GL_TEXTURE1);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, textureId2);
        std::cerr << "textureId2 : " << textureId2 << std::endl;
        glCheckError();
        mainShader.setUniformInt("texture2", 1);
        glCheckError();
        GLint texture1Location = glGetUniformLocation(mainShader.getShaderProgram(), "texture1");
        if (texture1Location == -1) {
            std::cerr << "Uniform 'texture1' not found or unused in shader." << std::endl;
        }

        GLint texture2Location = glGetUniformLocation(mainShader.getShaderProgram(), "texture2");
        if (texture2Location == -1) {
            std::cerr << "Uniform 'texture2' not found or unused in shader." << std::endl;
        }

        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "model"), 1, GL_FALSE, &model[0][0]);
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), "baseColor"), 1, &color[0]);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void render_to_framebuffer_cube(glm::vec3 color)
    {
        glViewport(0, 0, 800, 600);
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotationX, glm::vec3(1.0f, 0.0f, 0.0f));
        renderCube(color, projection, view, model);
    }

    void render_to_framebuffer_sphere(glm::vec3 color) {

        glViewport(0, 0, 800, 600);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model = glm::mat4(1.0f);

        renderSphere(projection, view, model, color);
    }

    void handleMouseInput()
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
            rotationY += mouseDelta.x * 0.005f;
            rotationX += mouseDelta.y * 0.005f;
            rotationX = glm::clamp(rotationX, -glm::half_pi<float>(), glm::half_pi<float>());
        }
    }

    void updateShaderAndConfiguration() {
        auto shaderCode = ShaderCodeGenerator::generateShaderCode(nodes_, graph_);

        mainShader.loadShaderFromString(shaderCode.vertexCode.c_str(), TypeShader::VERTEX_SHADER);
        mainShader.loadShaderFromString(shaderCode.fragmentCode.c_str(), TypeShader::FRAGMENT_SHADER);
        mainShader.createShaderProgram();

        std::cerr << shaderCode.fragmentCode << std::endl;
    }



    void show()
    {
        handleMouseInput();

        // Update timer context
        current_time_seconds = 0.001f *  getTicks();

        auto flags = ImGuiWindowFlags_MenuBar;

        // The node editor window
        ImGui::Begin("color node editor", NULL, flags);

        if (ImGui::BeginMenuBar())
        {

            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save"))
                {
                    // save_project("project.json");
                }
                if (ImGui::MenuItem("Load"))
                {
                    // load_project("project.json");
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Mini-map"))
            {
                const char* names[] = {
                    "Top Left",
                    "Top Right",
                    "Bottom Left",
                    "Bottom Right",
                };
                int locations[] = {
                    ImNodesMiniMapLocation_TopLeft,
                    ImNodesMiniMapLocation_TopRight,
                    ImNodesMiniMapLocation_BottomLeft,
                    ImNodesMiniMapLocation_BottomRight,
                };

                for (int i = 0; i < 4; i++)
                {
                    bool selected = minimap_location_ == locations[i];
                    if (ImGui::MenuItem(names[i], NULL, &selected))
                        minimap_location_ = locations[i];
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Style"))
            {
                if (ImGui::MenuItem("Classic"))
                {
                    ImGui::StyleColorsClassic();
                    ImNodes::StyleColorsClassic();
                }
                if (ImGui::MenuItem("Dark"))
                {
                    ImGui::StyleColorsDark();
                    ImNodes::StyleColorsDark();
                }
                if (ImGui::MenuItem("Light"))
                {
                    ImGui::StyleColorsLight();
                    ImNodes::StyleColorsLight();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::TextUnformatted("Edit the color of the output color window using nodes.");
        ImGui::Columns(2);
        ImGui::TextUnformatted("A -- add node");
        ImGui::TextUnformatted("X -- delete selected node or link");
        ImGui::NextColumn();
        if (ImGui::Checkbox("emulate_three_button_mouse", &emulate_three_button_mouse))
        {
            ImNodes::GetIO().EmulateThreeButtonMouse.Modifier =
                emulate_three_button_mouse ? &ImGui::GetIO().KeyAlt : NULL;
        }
        ImGui::Columns(1);

        ImNodes::BeginNodeEditor();

        // Handle new nodes
        // These are driven by the user, so we place this code before rendering the nodes
        {
            const bool open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                                    ImNodes::IsEditorHovered() && ImGui::IsKeyReleased(ImGuiKey_A);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
            if (!ImGui::IsAnyItemHovered() && open_popup)
            {
                ImGui::OpenPopup("add node");
            }

            if (ImGui::BeginPopup("add node"))
            {
                const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

                if (ImGui::MenuItem("add"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::add);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::add;
                    ui_node.ui.add.lhs = graph_.insert_node(value);
                    ui_node.ui.add.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    if(!graph_.edge_exists(ui_node.id, ui_node.ui.add.lhs)
                        || !graph_.edge_exists(ui_node.id, ui_node.ui.add.rhs)){
                        graph_.insert_edge(ui_node.id, ui_node.ui.add.lhs);
                        graph_.insert_edge(ui_node.id, ui_node.ui.add.rhs);
                    }
                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("multiply"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::multiply);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::multiply;
                    ui_node.ui.multiply.lhs = graph_.insert_node(value);
                    ui_node.ui.multiply.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.ui.multiply.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.ui.multiply.rhs);

                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("output") && root_node_id_ == -1)
                {
                    const Node value(NodeType::value, 0.f);
                    const Node out(NodeType::output);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::output;
                    ui_node.ui.output.r = graph_.insert_node(value);
                    ui_node.ui.output.g = graph_.insert_node(value);
                    ui_node.ui.output.b = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(out);

                    graph_.insert_edge(ui_node.id, ui_node.ui.output.r);
                    graph_.insert_edge(ui_node.id, ui_node.ui.output.g);
                    graph_.insert_edge(ui_node.id, ui_node.ui.output.b);

                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                    root_node_id_ = ui_node.id;
                }

                if (ImGui::MenuItem("sine"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::sine);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::sine;
                    ui_node.ui.sine.input = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.ui.sine.input);

                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("time"))
                {
                    UiNode ui_node;
                    ui_node.type = UiNodeType::time;
                    ui_node.id = graph_.insert_node(Node(NodeType::time));

                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("power"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::power);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::power;
                    ui_node.ui.power.lhs = graph_.insert_node(value);
                    ui_node.ui.power.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.ui.power.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.ui.power.rhs);

                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("cube viewport"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::cubeviewport);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::cubeviewport;
                    ui_node.ui.cubeviewport.input = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);
                    graph_.insert_edge(ui_node.id, ui_node.ui.cubeviewport.input);
                    nodes_.push_back(ui_node);
                    std::cout << "cube viewport node created with ID: " << ui_node.id << std::endl;
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }


                if (ImGui::MenuItem("sphere viewport"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::spherevieport);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::sphereviewport;
                    ui_node.ui.cubeviewport.input = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);
                    if(!graph_.edge_exists(ui_node.id, ui_node.ui.sphereviewport.input))
                    {
                        graph_.insert_edge(ui_node.id, ui_node.ui.sphereviewport.input);
                    }
                    nodes_.push_back(ui_node);
                    std::cout << "Sphere viewport node created with ID: " << ui_node.id << std::endl;
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("texture "))
                {
                    const Node textureNode(NodeType::texture);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::texture;
                    ui_node.ui.texture.id = -1;
                    ui_node.ui.texture.path = nullptr;

                    const char* file_filters[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tga" };
                    const char* selected_file = tinyfd_openFileDialog(
                        "Select Texture File", "", 5, file_filters, "Image Files (*.png, *.jpg)", 0);

                    if (selected_file)
                    {
                        ui_node.ui.texture.path = strdup(selected_file);
                        ui_node.ui.texture.id = textureManager.loadTexture(selected_file);
                    }

                    ui_node.id = graph_.insert_node(textureNode);
                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("blend")) {
                    const Node textureNode(NodeType::texture);
                    const Node value(NodeType::value, 0.5f);
                    UiNode ui_node;
                    ui_node.type = UiNodeType::blend;
                    //ui_node.ui.blend.id =  graph_.insert_node(value);
                    //ui_node.ui.blend.id2 =  graph_.insert_node(value);
                    ui_node.ui.blend.mixFactor = graph_.insert_node(value);

                    const char* file_filters[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tga" };
                    GLuint id,id2 = 0;

                    // Load Texture 1
                    const char* selected_file1 = tinyfd_openFileDialog(
                        "Select First Texture", "", 5, file_filters, "Image Files (*.png, *.jpg)", 0);
                    if (selected_file1) {
                        ui_node.ui.blend.path = strdup(selected_file1);
                        id = textureManager.loadTexture(selected_file1);
                        const Node value(NodeType::value, static_cast<float>(id));
                        ui_node.ui.blend.id =  graph_.insert_node(value);
                        std::cerr << "ui_node.ui.blend.id: " << ui_node.ui.blend.id << std::endl;
                        std::cerr << "Texture id: " << id << std::endl;

                    }

                    // Load Texture 2
                    const char* selected_file2 = tinyfd_openFileDialog(
                        "Select Second Texture", "", 5, file_filters, "Image Files (*.png, *.jpg)", 0);
                    if (selected_file2) {
                        ui_node.ui.blend.path2 = strdup(selected_file2);
                        id2 = textureManager.loadTexture(selected_file2);
                        const Node value(NodeType::value, id2);
                        ui_node.ui.blend.id2 =  graph_.insert_node(value);
                        std::cerr << "ui_node.ui.blend.id2: " << ui_node.ui.blend.id2 << std::endl;
                        std::cerr << "Texture id2: " << id2 << std::endl;
                    }


                    ui_node.id = graph_.insert_node(Node(NodeType::blend));

                    graph_.insert_edge(ui_node.id, ui_node.ui.blend.id);
                    graph_.insert_edge(ui_node.id, ui_node.ui.blend.id2);
                    graph_.insert_edge(ui_node.id, ui_node.ui.blend.mixFactor);

                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("adjustcolor")) {
                    UiNode ui_node;
                    ui_node.type = UiNodeType::colorAdjust;

                    ui_node.ui.colorAdjust.brightness = 0.0f; // Default brightness
                    ui_node.ui.colorAdjust.contrast = 1.0f;   // Default contrast
                    ui_node.ui.colorAdjust.saturation = 1.0f; // Default saturation

                    ui_node.id = graph_.insert_node(Node(NodeType::colorAdjust));
                    nodes_.push_back(ui_node);
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("light"))  {
                    UiNode node;
                    node.type = UiNodeType::light;
                    node.ui.pointLight.position = glm::vec3(10.0f, 10.0f, 10.0f);
                    node.ui.pointLight.color = glm::vec3(1.0f);
                    node.ui.pointLight.intensity = 1.0f;
                    node.id = graph_.insert_node(Node(NodeType::light));
                    nodes_.push_back(node);
                    ImNodes::SetNodeScreenSpacePos(node.id, click_pos);
                }

                /*if (ImGui::MenuItem("material"))  {
                    UiNode node;
                    node.type = UiNodeType::material;
                    node.ui.material.ambient = 0.1f;
                    node.ui.material.diffuse = 0.5f;
                    node.ui.material.specular = 0.5f;
                    node.ui.material.shininess = 32.0f;
                    node.id = graph_.insert_node(Node(NodeType::material));
                    nodes_.push_back(node);
                    ImNodes::SetNodeScreenSpacePos(node.id, click_pos);
                }*/


                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
        }

        for (UiNode& node : nodes_)
        {
            switch (node.type)
            {
            case UiNodeType::add:
            {
                const float node_width = 100.f;
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("add");
                ImNodes::EndNodeTitleBar();
                {
                    ImNodes::BeginInputAttribute(node.ui.add.lhs);
                    const float label_width = ImGui::CalcTextSize("left").x;
                    ImGui::TextUnformatted("left");
                    if (graph_.num_edges_from_node(node.ui.add.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.ui.add.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                {
                    ImNodes::BeginInputAttribute(node.ui.add.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.ui.add.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.ui.add.rhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    ImNodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("result").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("result");
                    ImNodes::EndOutputAttribute();
                }

                ImNodes::EndNode();
            }
            break;
            case UiNodeType::multiply:
            {
                const float node_width = 100.0f;
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("multiply");
                ImNodes::EndNodeTitleBar();

                {
                    ImNodes::BeginInputAttribute(node.ui.multiply.lhs);
                    const float label_width = ImGui::CalcTextSize("left").x;
                    ImGui::TextUnformatted("left");
                    if (graph_.num_edges_from_node(node.ui.multiply.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.ui.multiply.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                {
                    ImNodes::BeginInputAttribute(node.ui.multiply.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.ui.multiply.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.ui.multiply.rhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    ImNodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("result").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("result");
                    ImNodes::EndOutputAttribute();
                }

                ImNodes::EndNode();
            }
            break;
            case UiNodeType::output:
            {
                const float node_width = 100.0f;
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(45, 126, 194, 255));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(81, 148, 204, 255));
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("output");
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(node_width, 0.f));
                {
                    ImNodes::BeginInputAttribute(node.ui.output.r);
                    const float label_width = ImGui::CalcTextSize("r").x;
                    ImGui::TextUnformatted("r");
                    if (graph_.num_edges_from_node(node.ui.output.r) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.ui.output.r).value, 0.01f, 0.f, 1.0f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    ImNodes::BeginInputAttribute(node.ui.output.g);
                    const float label_width = ImGui::CalcTextSize("g").x;
                    ImGui::TextUnformatted("g");
                    if (graph_.num_edges_from_node(node.ui.output.g) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.ui.output.g).value, 0.01f, 0.f, 1.f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    ImNodes::BeginInputAttribute(node.ui.output.b);
                    const float label_width = ImGui::CalcTextSize("b").x;
                    ImGui::TextUnformatted("b");
                    if (graph_.num_edges_from_node(node.ui.output.b) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.ui.output.b).value, 0.01f, 0.f, 1.0f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }
                ImNodes::EndNode();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            break;
            case UiNodeType::sine:
            {
                const float node_width = 100.0f;
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("sine");
                ImNodes::EndNodeTitleBar();

                {
                    ImNodes::BeginInputAttribute(node.ui.sine.input);
                    const float label_width = ImGui::CalcTextSize("number").x;
                    ImGui::TextUnformatted("number");
                    if (graph_.num_edges_from_node(node.ui.sine.input) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel",
                            &graph_.node(node.ui.sine.input).value,
                            0.01f,
                            0.f,
                            1.0f);
                        ImGui::PopItemWidth();
                    }
                    ImNodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    ImNodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("output").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("output");
                    ImNodes::EndOutputAttribute();
                }

                ImNodes::EndNode();
            }
            break;
            case UiNodeType::time:
            {
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("time");
                ImNodes::EndNodeTitleBar();

                ImNodes::BeginOutputAttribute(node.id);
                ImGui::Text("output");
                ImNodes::EndOutputAttribute();

                ImNodes::EndNode();
            }
            break;
            case UiNodeType::power:
            {
                //const float node_width = 100.f;
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("Power");
                ImNodes::EndNodeTitleBar();

                ImNodes::BeginInputAttribute(node.ui.power.lhs);
                ImGui::TextUnformatted("Base");
                ImNodes::EndInputAttribute();

                ImNodes::BeginInputAttribute(node.ui.power.rhs);
                ImGui::TextUnformatted("Exponent");
                ImNodes::EndInputAttribute();

                ImNodes::BeginOutputAttribute(node.id);
                ImGui::TextUnformatted("Result");
                ImNodes::EndOutputAttribute();

                ImNodes::EndNode();
            }
            break;
            case UiNodeType::cubeviewport:
            {
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("Cube Viewport");
                ImNodes::EndNodeTitleBar();

                ImNodes::BeginInputAttribute(node.ui.cubeviewport.input);
                ImGui::TextUnformatted("Input");
                ImNodes::EndInputAttribute();

                glm::vec3 color(1.0f, 1.0f, 1.0f);

                if (graph_.num_edges_from_node(node.ui.cubeviewport.input) > 0)
                {
                    int input_id = graph_.node(node.ui.cubeviewport.input).value;
                    color.r = clamp(graph_.node(input_id).value, 0.0f, 1.0f);
                    color.g = clamp(graph_.node(input_id + 1).value, 0.0f, 1.0f);
                    color.b = clamp(graph_.node(input_id + 2).value, 0.0f, 1.0f);
;
                }

                unsigned int input_id = (unsigned int)graph_.node(node.ui.blend.id).value;
                textureId = graph_.node(input_id  + 1).value;
                textureId2= graph_.node(input_id + 2).value;
                mixFactor = graph_.node(input_id ).value;

                brightness = node.ui.colorAdjust.brightness;
                contrast = node.ui.colorAdjust.contrast;
                saturation = node.ui.colorAdjust.saturation;

                lightposition = node.ui.light.position;
                lightcolor  = node.ui.light.color;
                lightintensity = node.ui.light.intensity;

                frameBuffer.Bind();
                render_to_framebuffer_cube(color);
                ImGui::Image(reinterpret_cast<ImTextureID>(frameBuffer.getFrameTexture()), ImVec2(200, 200));
                frameBuffer.Unbind();
                ImNodes::EndNode();
            }
            break;
            case UiNodeType::sphereviewport:
            {
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("Sphere Viewport");
                ImNodes::EndNodeTitleBar();

                ImNodes::BeginInputAttribute(node.ui.sphereviewport.input);
                ImGui::TextUnformatted("Input");
                ImNodes::EndInputAttribute();

                glm::vec3 color(1.0f, 1.0f, 1.0f);
                if (graph_.num_edges_from_node(node.ui.sphereviewport.input) > 0)
                {
                    int input_id = graph_.node(node.ui.sphereviewport.input).value;
                    color.r = clamp(graph_.node(input_id).value, 0.0f, 1.0f);
                    color.g = clamp(graph_.node(input_id + 1).value, 0.0f, 1.0f);
                    color.b = clamp(graph_.node(input_id + 2).value, 0.0f, 1.0f);
                }

                frameBUfferSphere.Bind();
                render_to_framebuffer_sphere(color);
                ImGui::Image(reinterpret_cast<ImTextureID>(frameBUfferSphere.getFrameTexture()), ImVec2(200, 200));
                frameBUfferSphere.Unbind();
                ImNodes::EndNode();
            }
            break;
            case UiNodeType::texture:
            {
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("Texture Node");
                ImNodes::EndNodeTitleBar();

                if (node.ui.texture.path)
                {
                    //ImGui::Text("Texture: %s", node.ui.texture.path);
                    ImGui::Image(reinterpret_cast<ImTextureID>(node.ui.texture.id), ImVec2(100, 100));
                }
                else
                {
                    ImGui::Text("No texture selected");
                }

                ImNodes::BeginOutputAttribute(node.id);
                ImGui::TextUnformatted("Output");
                ImNodes::EndOutputAttribute();

                ImNodes::EndNode();
            }
            break;
            case UiNodeType::blend: {
                const float node_width = 100.0f;
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("Blend");
                ImNodes::EndNodeTitleBar();

                {
                    ImNodes::BeginInputAttribute((int)graph_.node(node.ui.blend.id).value);
                    ImGui::TextUnformatted("Texture 1");
                    ImNodes::EndInputAttribute();
                    ImGui::Image(reinterpret_cast<ImTextureID>((int)graph_.node(node.ui.blend.id).value), ImVec2(100, 100));
                }

                {
                    ImNodes::BeginInputAttribute((int)graph_.node(node.ui.blend.id2).value);
                    ImGui::TextUnformatted("Texture 2");
                    ImNodes::EndInputAttribute();
                    ImGui::Image(reinterpret_cast<ImTextureID>((int)graph_.node(node.ui.blend.id2).value), ImVec2(100, 100));
                }

                ImGui::Spacing();

                ImGui::PushItemWidth(node_width);
                ImGui::DragFloat("Mix Factor", &graph_.node(node.ui.blend.mixFactor).value, 0.01f, 0.0f, 2.0f);
                ImGui::PopItemWidth();

                ImNodes::BeginOutputAttribute(node.id);
                ImGui::TextUnformatted("Blended Output");
                ImNodes::EndOutputAttribute();

                ImNodes::EndNode();
            } break;

            case UiNodeType::colorAdjust: {
                const float node_width = 150.0f;
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("Color Adjust");
                ImNodes::EndNodeTitleBar();

                ImGui::PushItemWidth(node_width);
                ImGui::DragFloat("Brightness", &node.ui.colorAdjust.brightness, 0.01f, -1.0f, 1.0f);
                ImGui::DragFloat("Contrast", &node.ui.colorAdjust.contrast, 0.01f, 0.0f, 2.0f);
                ImGui::DragFloat("Saturation", &node.ui.colorAdjust.saturation, 0.01f, 0.0f, 2.0f);
                ImGui::PopItemWidth();

                ImNodes::BeginOutputAttribute(node.id);
                ImGui::TextUnformatted("Adjusted Output");
                ImNodes::EndOutputAttribute();

                ImNodes::EndNode();
            }
            break;
            case UiNodeType::light: {
                ImNodes::BeginNode(node.id);

                ImNodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("Light Node");
                ImNodes::EndNodeTitleBar();

                ImGui::DragFloat3("Position", &node.ui.light.position.x, 0.1f);
                ImGui::ColorEdit3("Color", &node.ui.light.color.x);
                ImGui::DragFloat("Intensity", &node.ui.light.intensity, 0.1f, 0.0f, 10.0f);

                ImNodes::BeginOutputAttribute(node.id);
                ImGui::TextUnformatted("Output");
                ImNodes::EndOutputAttribute();

                ImNodes::EndNode();
            }
            break;

                /*case UiNodeType::light: {
                    ImNodes::BeginNode(node.id);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::TextUnformatted("Point Light");
                    ImNodes::EndNodeTitleBar();

                    ImGui::ColorEdit3("Color", &node.ui.pointLight.color.x);
                    ImGui::DragFloat3("Position", &node.ui.pointLight.position.x, 0.1f);
                    ImGui::DragFloat("Intensity", &node.ui.pointLight.intensity, 0.01f, 0.0f, 10.0f);
                    ImGui::DragFloat("Radius", &node.ui.pointLight.radius, 0.1f, 0.0f, 100.0f);
                    ImGui::DragFloat("Attenuation", &node.ui.pointLight.attenuation, 0.01f, 0.0f, 2.0f);

                    ImNodes::BeginOutputAttribute(node.id);
                    ImGui::TextUnformatted("Light Output");
                    ImNodes::EndOutputAttribute();

                    ImNodes::EndNode();
                }
            break;
             case UiNodeType::material: {
                    ImNodes::BeginNode(node.id);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::TextUnformatted("Material");
                    ImNodes::EndNodeTitleBar();

                    ImGui::DragFloat("Ambient", &node.ui.material.ambient, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Diffuse", &node.ui.material.diffuse, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Specular", &node.ui.material.specular, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Shininess", &node.ui.material.shininess, 1.0f, 1.0f, 256.0f);

                    ImNodes::BeginOutputAttribute(node.id);
                    ImGui::TextUnformatted("Material Output");
                    ImNodes::EndOutputAttribute();

                    ImNodes::EndNode();
                }



            break;

             case UiNodeType::pointLight:
             case UiNodeType::directionalLight:
             case UiNodeType::spotLight:
             case UiNodeType::lightingModel:
                 break;*/
            }
        }

        for (const auto& edge : graph_.edges())
        {
            // If edge doesn't start at value, then it's an internal edge, i.e.
            // an edge which links a node's operation to its input. We don't
            // want to render node internals with visible links.
            if (graph_.node(edge.from).type != NodeType::value)
                continue;

            ImNodes::Link(edge.id, edge.from, edge.to);
        }

        for(const auto& edge : graph_.edges()) {
            if (!graph_.edge_exists(edge.from, edge.to)) {
                graph_.insert_edge(edge.from, edge.
                                              to);
            }
        }

        ImNodes::MiniMap(0.2f, minimap_location_);
        ImNodes::EndNodeEditor();

        // Handle new links
        // These are driven by Imnodes, so we place the code after EndNodeEditor().

        {
            int start_attr, end_attr;
            if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
            {
                const NodeType start_type = graph_.node(start_attr).type;
                const NodeType end_type = graph_.node(end_attr).type;

                const bool valid_link = start_type != end_type;
                if (valid_link)
                {
                    // Ensure the edge is always directed from the value to
                    // whatever produces the value
                    if (start_type != NodeType::value)
                    {
                        std::swap(start_attr, end_attr);
                    }
                    graph_.insert_edge(start_attr, end_attr);
                }
            }
        }

        // Handle deleted links

        {
            int link_id;
            if (ImNodes::IsLinkDestroyed(&link_id))
            {
                graph_.erase_edge(link_id);
            }
        }

        {
            const int num_selected = ImNodes::NumSelectedLinks();
            if (num_selected > 0 && ImGui::IsKeyReleased(ImGuiKey_X))
            {
                static std::vector<int> selected_links;
                selected_links.resize(static_cast<size_t>(num_selected));
                ImNodes::GetSelectedLinks(selected_links.data());
                for (const int edge_id : selected_links)
                {
                    graph_.erase_edge(edge_id);
                }
            }
        }

        {
            const int num_selected = ImNodes::NumSelectedNodes();
            if (num_selected > 0 && ImGui::IsKeyReleased(ImGuiKey_X))
            {
                static std::vector<int> selected_nodes;
                selected_nodes.resize(static_cast<size_t>(num_selected));
                ImNodes::GetSelectedNodes(selected_nodes.data());
                for (const int node_id : selected_nodes)
                {
                    graph_.erase_node(node_id);
                    auto iter = std::find_if(
                        nodes_.begin(), nodes_.end(), [node_id](const UiNode& node) -> bool {
                            return node.id == node_id;
                        });
                    // Erase any additional internal nodes
                    switch (iter->type)
                    {
                    case UiNodeType::add:
                        graph_.erase_node(iter->ui.add.lhs);
                        graph_.erase_node(iter->ui.add.rhs);
                        break;
                    case UiNodeType::multiply:
                        graph_.erase_node(iter->ui.multiply.lhs);
                        graph_.erase_node(iter->ui.multiply.rhs);
                        break;
                    case UiNodeType::output:
                        graph_.erase_node(iter->ui.output.r);
                        graph_.erase_node(iter->ui.output.g);
                        graph_.erase_node(iter->ui.output.b);
                        root_node_id_ = -1;
                        break;
                    case UiNodeType::sine:
                        graph_.erase_node(iter->ui.sine.input);
                        break;
                    case UiNodeType::power:
                        graph_.erase_node(iter->ui.power.lhs);
                        graph_.erase_node(iter->ui.power.rhs);
                        break;
                    case UiNodeType::cubeviewport:
                        graph_.erase_node(iter->ui.cubeviewport.input);
                        break;
                    case UiNodeType::sphereviewport:
                        graph_.erase_node(iter->ui.sphereviewport.input);
                        break;
                    case UiNodeType::texture:
                        graph_.erase_node(iter->ui.texture.id);
                        break;

                    default:
                        break;
                    }
                    nodes_.erase(iter);
                }
            }
        }

        ImGui::End();

        // The color output window

        const ImU32 color =
            root_node_id_ != -1 ? evaluate(graph_, root_node_id_) : IM_COL32(255, 20, 147, 255);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
        ImGui::Begin("output color");
        ImGui::End();
        ImGui::PopStyleColor();

        ImGui::Begin("3d view");
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        ImGui::Image(reinterpret_cast<ImTextureID>(frameBuffer.getFrameTexture()), windowSize);
        ImGui::End();

        if(ImGui::Button("compile")) {
            updateShaderAndConfiguration();
        }

        //for (const auto& edge : graph_.edges()) {
        //    std::cerr << "Edge: from = " << edge.from << ", to = " << edge.to << std::endl;
        //}

        for (const auto& edge : graph_.edges()) {
            if (edge.from == edge.to) {
                std::cerr << "Removing self-loop edge: " << edge.id << std::endl;
                graph_.erase_edge(edge.id);
            }
        }

    }


    ImU32 evaluate(const Graph<Node>& graph, const int root_node)
    {
        std::stack<int> postorder;
        dfs_traverse(
            graph, root_node, [&postorder](const int node_id) -> void { postorder.push(node_id); });

        std::stack<float> value_stack;
        while (!postorder.empty())
        {
            const int id = postorder.top();
            postorder.pop();
            const Node node = graph.node(id);

            switch (node.type)
            {
            case NodeType::add:
            {
                const float rhs = value_stack.top();
                value_stack.pop();
                const float lhs = value_stack.top();
                value_stack.pop();
                value_stack.push(lhs + rhs);
            }
            break;
            case NodeType::multiply:
            {
                const float rhs = value_stack.top();
                value_stack.pop();
                const float lhs = value_stack.top();
                value_stack.pop();
                value_stack.push(rhs * lhs);
            }
            break;
            case NodeType::sine:
            {
                const float x = value_stack.top();
                value_stack.pop();
                const float res = std::abs(std::sin(x));
                value_stack.push(res);
            }
            break;
            case NodeType::time:
            {
                value_stack.push(current_time_seconds);
            }
            break;
            case NodeType::value:
            {
                // If the edge does not have an edge connecting to another node, then just use the value
                // at this node. It means the node's input pin has not been connected to anything and
                // the value comes from the node's UI.
                if (graph.num_edges_from_node(id) == 0ull)
                {
                    value_stack.push(node.value);
                }
            }
            break;
            case NodeType::power:
            {
                const float rhs = value_stack.top();
                value_stack.pop();
                const float lhs = value_stack.top();
                value_stack.pop();
                value_stack.push(std::pow(lhs, rhs));
            }
            break;
            case NodeType::texture:
            {
                const float id = value_stack.top();
                value_stack.pop();
                value_stack.push(static_cast<float>(id));
            }
            break;
            case NodeType::blend:
            {
                const float texture1 = value_stack.top(); // First texture ID
                value_stack.pop();
                const float texture2 = value_stack.top(); // Second texture ID
                value_stack.pop();
                const float mixFactor = node.value; // Assume mixFactor is stored in the node's value
                value_stack.push(texture1 * (1.0f - mixFactor) + texture2 * mixFactor);
            }
            break;
            case NodeType::colorAdjust:
            {
                const float saturation = node.value; // Assume saturation is stored in node.value
                const float contrast = value_stack.top();
                value_stack.pop();
                const float brightness = value_stack.top();
                value_stack.pop();
                float adjusted = value_stack.top(); // Base color
                value_stack.pop();

                adjusted += brightness;
                adjusted = (adjusted - 0.5f) * contrast + 0.5f;
                adjusted = std::min(1.0f, std::max(0.0f, adjusted)); // Clamp to [0, 1]
                value_stack.push(adjusted * saturation);
            }
            break;
            default:
                break;
            }
        }

        // The final output node isn't evaluated in the loop -- instead we just pop
        // the three values which should be in the stack.
        assert(value_stack.size() == 3ull);
        const int b = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
        value_stack.pop();
        const int g = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
        value_stack.pop();
        const int r = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
        value_stack.pop();

        return IM_COL32(r, g, b, 255);
    }
};


