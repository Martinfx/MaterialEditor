#pragma once
#include <iostream>
#include <imgui.h>
#include <imnodes.h>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include <GL/glew.h>
#include "3rdparty/tinyfiledialogs/include/tinyfiledialogs/tinyfiledialogs.h"
#include "node.h"
#include "link.h"
#include "object.hpp"
#include "shader.hpp"
#include "debug.hpp"
#include "framebuffer.hpp"
#include "texture.hpp"

struct SerializableNode {
    int id_;
    NodeType type_;
    float position_x_;
    float position_y_;
    std::vector<Pin*> pins_;
    nlohmann::json data_;
};

struct SerializableLink {
    int id_;
    int input_slot_id_;
    int output_slot_id_;

    friend void to_json(nlohmann::json& j, const SerializableLink& link) {
        j = nlohmann::json{
                           {"id", link.id_},
                           {"input_slot_id", link.input_slot_id_},
                           {"output_slot_id", link.output_slot_id_}};
    }

    friend void from_json(const nlohmann::json& j, SerializableLink& link) {
        j.at("id").get_to(link.id_);
        j.at("input_slot_id").get_to(link.input_slot_id_);
        j.at("output_slot_id").get_to(link.output_slot_id_);
    }
};


struct EditorState {
    std::vector<SerializableNode> nodes;
    std::vector<SerializableLink> links;
};

void to_json(nlohmann::json& j, const SerializableNode& node) {
    j = nlohmann::json{
        {"id", node.id_},
        {"type", node.type_},
        {"position_x", node.position_x_},
        {"position_y", node.position_y_},
        {"data", node.data_}
    };
}

void from_json(const nlohmann::json& j, SerializableNode& node) {
    j.at("id").get_to(node.id_);
    j.at("type").get_to(node.type_);
    j.at("position_x").get_to(node.position_x_);
    j.at("position_y").get_to(node.position_y_);
    j.at("data").get_to(node.data_);
}

void to_json(nlohmann::json& j, const EditorState& state) {
    j = nlohmann::json{
        {"nodes", state.nodes},
        {"links", state.links}
    };
}

void from_json(const nlohmann::json& j, EditorState& state) {
    j.at("nodes").get_to(state.nodes);
    j.at("links").get_to(state.links);
}


class NodeEditor {
public:
    NodeEditor();
    ~NodeEditor();

    void render();
    void add_node(Node* node);
    void add_link(int input_pin, int output_pin, Node* input_node, Node* output_node);
    void handle_link_creation();
    void handle_link_deletion();
    void handle_deletion();
    void remove_selected_nodes();

    void save(const std::string& filename);
    void load(const std::string& filename);

    std::string generateShaderCode();
    void updateShader();
    std::vector<Node*> topologicalSort();

private:

    unsigned int fbo = 0;
    unsigned int texture = 0;
    unsigned int rbo, ebo = 0;
    unsigned int cubeVAO, cubeVBO = 0;
    unsigned int sphereVAO, sphereVBO, sphereEBO = 0;

    bool initialized =     false;

    float rotationY = 0;
    float rotationX = 0;

    // blend
    GLuint textureId = 0, textureId2 = 0;
    float mixFactor = 0.7f;
    // adjust
    float brightness = 0.0f, contrast =  0.0f, saturation = 0.0f;

    Shader mainShader;
    FrameBuffer frameBuffer;
    TextureManager textureManager;

    ImNodesMiniMapLocation minimap_location_;
    std::vector<Node*> nodes_;
    std::vector<Link*> links_;
    int hovered_link_id_;
    int selected_node_id_;

    void remove_link(int unique_link_id);
    void render_elements();
    void remove_node(int node_id);
    void update_links();
    EditorState get_state() const;
    void set_state(const EditorState& state);

    void render_viewport(glm::vec3 &color);
    void handleMouseInput();
};

NodeEditor::NodeEditor() : hovered_link_id_(-1), selected_node_id_(-1), minimap_location_(ImNodesMiniMapLocation_BottomRight)
{
    ImNodes::StyleColorsDark();
    ImNodes::SetCurrentContext(ImNodes::CreateContext());

    frameBuffer.InitFrameBuffer(800,600);
    glCheckError();

    auto shaderCode = generateShaderCode();
    mainShader.loadShader("vertex_shader.glsl", TypeShader::VERTEX_SHADER);
    mainShader.loadShaderFromString(shaderCode.c_str(), TypeShader::FRAGMENT_SHADER);
    mainShader.createShaderProgram();

}

NodeEditor::~NodeEditor() {
    for (auto node : nodes_) {
        delete node;
    }
    for (auto link : links_) {
        delete link;
    }
    nodes_.clear();
    links_.clear();
}

void NodeEditor::handleMouseInput()
{
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
        rotationY += mouseDelta.x * 0.005f;
        rotationX += mouseDelta.y * 0.005f;
        rotationX = glm::clamp(rotationX, -glm::half_pi<float>(), glm::half_pi<float>());
    }
}

void NodeEditor::render_viewport(glm::vec3 &color) {

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


    glActiveTexture(GL_TEXTURE0);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, textureId);
    std::cerr << "textureId : " << textureId << std::endl;
    glCheckError();
    mainShader.setUniformInt("tex0", 0);
    glCheckError();

    glActiveTexture(GL_TEXTURE1);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, textureId2);
    std::cerr << "textureId2 : " << textureId2 << std::endl;
    glCheckError();
    mainShader.setUniformInt("tex1", 1);
    glCheckError();
    GLint texture1Location = glGetUniformLocation(mainShader.getShaderProgram(), "tex0");
    if (texture1Location == -1) {
        std::cerr << "Uniform 'tex0' not found or unused in shader." << std::endl;
    }

    GLint texture2Location = glGetUniformLocation(mainShader.getShaderProgram(), "tex1");
    if (texture2Location == -1) {
        std::cerr << "Uniform 'tex1' not found or unused in shader." << std::endl;
    }

    glCheckError();
    glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "mixFactor"), mixFactor);
    std::cerr << "mixfactor : " << mixFactor << std::endl;
    glCheckError();
    glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "brightness"), brightness);
    glCheckError();
    std::cerr << "brightness: " << brightness << std::endl;
    glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "contrast"), contrast);
    glCheckError();
    std::cerr << "contrast: " << contrast << std::endl;
    glUniform1f(glGetUniformLocation(mainShader.getShaderProgram(), "saturation"), saturation);
    glCheckError();
    std::cerr << "saturation: " << saturation << std::endl;

    glBindVertexArray(cubeVAO);
    glCheckError();

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glCheckError();
    glBindVertexArray(0);
    glCheckError();
}

std::string NodeEditor::generateShaderCode() {
    std::stringstream shaderCode;

    shaderCode << "#version 330 core\n\n";
    shaderCode << "in vec2 TexCoords;\n";
    shaderCode << "in vec3 Normal;\n";
    shaderCode << "in vec3 FragPos;\n";
    shaderCode << "out vec4 FragColor;\n\n";
    shaderCode << "uniform sampler2D tex0;\n";
    shaderCode << "uniform sampler2D tex1;\n";
    shaderCode << "uniform float mixFactor;\n";
    shaderCode << "uniform float brightness;\n";
    shaderCode << "uniform float contrast;\n";
    shaderCode << "uniform float saturation;\n";

    shaderCode << R"(
    vec3 adjustColor(vec3 color, float brightness, float contrast, float saturation) {
        color += brightness;
        color = (color - 0.5) * contrast + 0.5;
        float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
        return mix(vec3(luminance), color, saturation);
    }

    vec4 blendTextures(vec4 tex1, vec4 tex2, float factor) {
        return mix(tex1, tex2, factor);
    }
    )";

    shaderCode << "void main() {\n";
    shaderCode << "    vec3 color = vec3(1.0f);\n";

    auto sortedNodes = topologicalSort();
    for (const auto& node : sortedNodes) {
        if (node->get_type() == NodeType::Blend) {
            shaderCode << "    vec4 blendResult = blendTextures(texture(tex0, TexCoords), texture(tex1, TexCoords), mixFactor);\n";
            shaderCode << "    color *= blendResult.rgb;\n";
        } else if (node->get_type() == NodeType::ColorAdjust) {
            shaderCode << "    color *= adjustColor(color, brightness, contrast, saturation);\n";
        }
    }


    shaderCode <<"  FragColor = vec4(color, 1.0); }\n";

    return shaderCode.str();
}

void NodeEditor::updateShader() {
    std::string fragmentShaderCode = generateShaderCode();
    std::cerr << fragmentShaderCode << std::endl;
    mainShader.loadShader("vertex_shader.glsl", TypeShader::VERTEX_SHADER);
    mainShader.loadShaderFromString(fragmentShaderCode.c_str(), TypeShader::FRAGMENT_SHADER);
    mainShader.createShaderProgram();

    std::cout << "Shader recompiled successfully." << std::endl;
}

std::vector<Node*> NodeEditor::topologicalSort() {
    std::unordered_map<int, std::vector<int>> graph; // Mapování z node ID na seznam sousedů
    std::unordered_map<int, int> indegree;          // Počet vstupních hran pro každý uzel

    for (const auto& link : links_) {
        graph[link->output_node_->get_unique_id()].push_back(link->input_node_->get_unique_id());
        indegree[link->input_node_->get_unique_id()]++;
    }

    std::queue<Node*> queue;
    for (const auto& node : nodes_) {
        if (indegree[node->get_unique_id()] == 0) {
            queue.push(node);
        }
    }

    std::vector<Node*> sortedNodes;
    while (!queue.empty()) {
        Node* current = queue.front();
        queue.pop();
        sortedNodes.push_back(current);

        for (int neighbor : graph[current->get_unique_id()]) {
            indegree[neighbor]--;
            if (indegree[neighbor] == 0) {
                auto it = std::find_if(nodes_.begin(), nodes_.end(),
                                       [neighbor](Node* n) { return n->get_unique_id() == neighbor; });
                if (it != nodes_.end()) {
                    queue.push(*it);
                }
            }
        }
    }

    return sortedNodes;
}

void NodeEditor::render() {
    handleMouseInput();
    ////////////////////////////////
    glm::vec3 cube_color(1.0f, 1.0f, 1.0f);


    frameBuffer.Bind();
    render_viewport(cube_color);
    frameBuffer.Unbind();

    ////////////////////////////////
    auto flags = ImGuiWindowFlags_MenuBar;

    // The node editor window
    ImGui::Begin("color node editor", NULL, flags);

    if (ImGui::BeginMenuBar())
    {

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                save("save.json");
            }
            if (ImGui::MenuItem("Load"))
            {
                load("save.json");
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

        if(ImGui::BeginMenu("Compile"))
        {
            if (ImGui::MenuItem("update shaders"))
            {
                updateShader();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::TextUnformatted("Edit the color of the output color window using nodes.");
    ImGui::Columns(2);
    ImGui::TextUnformatted("A -- add node");
    ImGui::TextUnformatted("X -- delete selected node or link");
    ImGui::End();

    ImNodes::BeginNodeEditor();

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

        if (ImGui::MenuItem("CubeViewport"))
        {
            auto node = new CubeViewportNode(glm::vec2(10, 10));
            node->set_framebuffer(frameBuffer.getFrameBuffer(), frameBuffer.getFrameTexture(), texture);
            nodes_.push_back(node);
        }

        if (ImGui::MenuItem("Blend"))
        {
            auto node = new TextureBlendNode(glm::vec2(10, 10));
            const char* file_filters[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tga" };
            // Load Texture 1
            const char* selected_file1 = tinyfd_openFileDialog(
                "Select First Texture", "", 5, file_filters, "Image Files (*.png, *.jpg)", 0);


            if (selected_file1) {
                textureId = textureManager.loadTexture(selected_file1);

                std::cerr << "Texture id: " << textureId << std::endl;

            }

            // Load Texture 2
            const char* selected_file2 = tinyfd_openFileDialog(
                "Select Second Texture", "", 5, file_filters, "Image Files (*.png, *.jpg)", 0);
            if (selected_file2) {
                textureId2 = textureManager.loadTexture(selected_file2);
                std::cerr << "Texture id2: " << textureId2 << std::endl;
            }
            node->setTextures(textureId, textureId2);
            nodes_.push_back(node);
        }

        if (ImGui::MenuItem("AdjustColor"))
        {
            auto node = new ColorAdjustNode(glm::vec2(10, 10));
            nodes_.push_back(node);
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();

    render_elements();
    ImNodes::MiniMap(0.2f, minimap_location_);
    ImNodes::EndNodeEditor();

    ImGui::Begin("3d view");
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    frameBuffer.Bind();
    glCheckError();
    ImGui::Image(static_cast<ImTextureID>(frameBuffer.getFrameTexture()), windowSize);
    frameBuffer.Unbind();
    glCheckError();
    ImGui::End();


    handle_link_creation();
    handle_link_deletion();
}

void NodeEditor::render_elements() {
    for (auto& node : nodes_) {
        node->draw_node();
    }
    for (auto& link : links_) {
        ImNodes::Link(link->id_, link->input_slot_id_, link->output_slot_id_);
    }
}

void NodeEditor::add_node(Node* node) {
    nodes_.push_back(node);
}

void NodeEditor::add_link(int input_pin, int output_pin, Node* input_node, Node* output_node) {
    links_.push_back(new Link(input_pin, output_pin, input_node, output_node));
}

void NodeEditor::handle_link_creation() {
    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        Node* input_node = nullptr;
        Node* output_node = nullptr;

        for (auto& node : nodes_) {
            for (auto& pin : node->get_pins()) {
                if (pin->unique_id_ == start_attr) {
                    input_node = node;
                }
                if (pin->unique_id_ == end_attr) {
                    output_node = node;
                }
            }
        }

        if (input_node && output_node) {
            add_link(start_attr, end_attr, input_node, output_node);
            output_node->inputs_.push_back(input_node->get_unique_id());
        }
    }

    if (ImNodes::IsLinkHovered(&hovered_link_id_)) {
        if (ImGui::IsMouseClicked(0)) {
            std::cout << "Clicked on link ID: " << hovered_link_id_ << std::endl;
        }
    }

    for (auto& node : nodes_) {
        int node_id = node->get_unique_id();
        if (ImNodes::IsNodeHovered(&node_id)) {
            if (ImGui::IsMouseClicked(0)) {
                selected_node_id_ = node->get_unique_id();
                std::cout << "Selected node ID: " << selected_node_id_ << std::endl;
            }
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_X)) {
        remove_selected_nodes();
    }
}

void NodeEditor::handle_deletion() {
    if (hovered_link_id_ != -1 && ImGui::IsKeyPressed(ImGuiKey_X)) {
        remove_link(hovered_link_id_);
        hovered_link_id_ = -1;
    }

    if (selected_node_id_ != -1 && ImGui::IsKeyPressed(ImGuiKey_X)) {
        remove_node(selected_node_id_);
        selected_node_id_ = -1;
    }
}

void NodeEditor::remove_node(int node_id) {
    auto it = std::find_if(nodes_.begin(), nodes_.end(), [node_id](Node* node) {
        return node->get_unique_id() == node_id;
    });

    if (it != nodes_.end()) {

        for (auto pin : (*it)->get_pins()) {
            remove_link(pin->unique_id_);
        }

        delete *it;
        nodes_.erase(it);
    }
}

void NodeEditor::handle_link_deletion() {
    if (hovered_link_id_ != -1 && ImGui::IsKeyPressed(ImGuiKey_X)) {
        remove_link(hovered_link_id_);
        hovered_link_id_ = -1;
    }
}

void NodeEditor::remove_link(int unique_link_id) {
    auto it = std::find_if(links_.begin(), links_.end(), [unique_link_id](Link* link) {
        return link->id_ == unique_link_id;
    });

    if (it != links_.end()) {
        delete *it;
        links_.erase(it);
    }
}

void NodeEditor::remove_selected_nodes() {
    const int num_selected_nodes = ImNodes::NumSelectedNodes();
    if (num_selected_nodes > 0) {
        std::vector<int> selected_node_ids(num_selected_nodes);
        ImNodes::GetSelectedNodes(selected_node_ids.data());

        for (int node_id : selected_node_ids) {
            remove_node(node_id);
        }
    }
}

void NodeEditor::update_links() {
    for (auto& link : links_) {
        link->transfer_data();
    }
}

void NodeEditor::save(const std::string& filename) {
    EditorState state;

    for (const auto& node : nodes_) {
        SerializableNode s_node;
        s_node.id_ = node->get_unique_id();
        s_node.type_ = node->get_type();
        s_node.position_x_ = node->position_.x;
        s_node.position_y_ = node->position_.y;
        s_node.data_ = node->get_data().to_json();

        state.nodes.push_back(s_node);
    }

    for (const auto& link : links_) {
        SerializableLink s_link;
        s_link.id_ = link->id_;
        s_link.input_slot_id_ = link->input_slot_id_;
        s_link.output_slot_id_ = link->output_slot_id_;

        state.links.push_back(s_link);
    }

    nlohmann::json j = state;
    std::ofstream file(filename);
    file << j.dump(4);
    file.close();
}

void NodeEditor::load(const std::string& filename) {

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    nlohmann::json j;
    file >> j;
    file.close();

    EditorState state;
    j.get_to(state);

    nodes_.clear();
    links_.clear();

    for (const auto& s_node : state.nodes) {
        Node* node = nullptr;
        if (s_node.type_ == NodeType::Blend) {
            node = new TextureBlendNode(glm::vec2(s_node.position_x_, s_node.position_y_));
        } else if (s_node.type_ == NodeType::ColorAdjust) {
            node = new ColorAdjustNode(glm::vec2(s_node.position_x_, s_node.position_y_));
        }else if (s_node.type_ == NodeType::CubeViewer) {
            node = new CubeViewportNode(glm::vec2(s_node.position_x_, s_node.position_y_));
        }

        if (node) {
            node->set_data(s_node.data_.get<NodeData>());
            nodes_.push_back(node);
        }
    }

    for (const auto& s_link : state.links) {
        Node* input_node = nullptr;
        Node* output_node = nullptr;

        for (auto& node : nodes_) {
            for (const auto& pin : node->get_pins()) {
                if (pin->unique_id_ == s_link.input_slot_id_) {
                    input_node = node;
                }
                if (pin->unique_id_ == s_link.output_slot_id_) {
                    output_node = node;
                }
            }
        }

        if (input_node && output_node) {
            links_.push_back(new Link(s_link.input_slot_id_, s_link.output_slot_id_, input_node, output_node));
        } else {
            std::cerr << "Error: Could not find nodes for link: " << s_link.id_ << std::endl;
        }
    }
}
