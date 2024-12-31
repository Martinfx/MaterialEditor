#pragma once
#include <imgui.h>
#include <imnodes.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <GL/glew.h>

enum NodeType {
    Blend,
    ColorAdjust,
    CubeViewer,
    None
};

enum PinType {
    Out,
    In
};

struct Pin {
    Pin() {}
    Pin(NodeType compatibility, PinType pin_type, int nb_connexions);
    int unique_id_;
    int nb_connexions_;
    PinType type_;
    NodeType compatibility_;

    friend void to_json(nlohmann::json& j, const Pin& p);
    friend void from_json(const nlohmann::json& j, Pin& p);
};

struct NodeData {
    float value;
    NodeData() : value(0.0f) {}

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"value", value}
        };
    }

    friend void to_json(nlohmann::json& j, const NodeData& data) {
        j = nlohmann::json{{"value", data.value}};
    }

    friend void from_json(const nlohmann::json& j, NodeData& data) {
        j.at("value").get_to(data.value);
    }
};

class Node {
public:
    Node() {}
    Node(glm::vec2 &position, NodeType node_type);
    virtual ~Node();

    void virtual draw_node() = 0;

    const int& get_unique_id() const { return id_; }
    std::vector<Pin*>& get_pins() { return pins_; }
    NodeType& get_type() { return type_; }
    NodeData get_data() const { return data_; }
    void set_data(const NodeData& data) { data_ = data; }

    float get_position_x() const { return position_.x; }
    float get_position_y() const { return position_.y; }

    void set_position_x(float x) { position_.x = x; }
    void set_position_y(float y) { position_.y = y; }

    // Serialization and deserialization functions
    friend void to_json(nlohmann::json& j, const Node& n);
    friend void from_json(const nlohmann::json& j, Node& n);

    std::vector<int> connectedInputIds;
    std::vector<int> connectedOutputIds;
    bool isConnected() const {
        return !connectedInputIds.empty() || !connectedOutputIds.empty();
    }

    std::vector<int> inputs_;
    NodeType type_;
    std::vector<Pin*> pins_;
    glm::vec2 position_;
    int id_;
    NodeData data_;
};

Pin::Pin(NodeType compatibility, PinType pin_type, int nb_connexions)
    : compatibility_(compatibility), type_(pin_type), nb_connexions_(nb_connexions) {
    static int unique_id_counter = 0;
    unique_id_ = unique_id_counter++;
}

Node::Node(glm::vec2& position, NodeType node_type)
    : position_(position), type_(node_type) {
    static int unique_id_counter = 0;
    id_ = unique_id_counter++;
}

Node::~Node() {
    for (auto pin : pins_) {
        delete pin;
    }
}

class TextureBlendNode : public Node {
public:
    TextureBlendNode(glm::vec2 position = glm::vec2(0, 0));
    void draw_node() override;

    void setTextures(GLuint tex1, GLuint tex2) {
        texture1 = tex1;
        texture2 = tex2;
    }

private:
    float mix_factor_ = 0.5f;
    GLuint texture1, texture2 = 0;
};

TextureBlendNode::TextureBlendNode(glm::vec2 position)
    : Node(position, NodeType::Blend) {
    pins_.push_back(new Pin(None, In, 2));
    pins_.push_back(new Pin(ColorAdjust, Out, 1));
}

void TextureBlendNode::draw_node() {
    const float node_width = 100.0f;
    ImNodes::BeginNode(get_unique_id());

    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Texture Blend");
    ImNodes::EndNodeTitleBar();

    {
        ImNodes::BeginInputAttribute(pins_[0]->unique_id_);
        ImGui::TextUnformatted("Texture 1");
        ImNodes::EndInputAttribute();
        ImGui::Image(reinterpret_cast<ImTextureID>(texture1), ImVec2(100, 100));
    }

    {
        ImNodes::BeginInputAttribute(pins_[1]->unique_id_);
        ImGui::TextUnformatted("Texture 2");
        ImNodes::EndInputAttribute();
        ImGui::Image(reinterpret_cast<ImTextureID>(texture2), ImVec2(100, 100));
    }

    ImGui::PushItemWidth(node_width);
    ImGui::DragFloat("Mix Factor", &mix_factor_, 0.01f, 0.0f, 1.0f);
    ImGui::PopItemWidth();


    ImNodes::BeginOutputAttribute(pins_[1]->unique_id_);
    ImGui::Text("Blended Texture");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}


class ColorAdjustNode : public Node {
public:
    ColorAdjustNode(glm::vec2 position = glm::vec2(0, 0));
    void draw_node() override;

private:
    float brightness_ = 0.0f;
    float contrast_ = 1.0f;
    float saturation_ = 1.0f;
};

ColorAdjustNode::ColorAdjustNode(glm::vec2 position)
    : Node(position, NodeType::ColorAdjust) {
    pins_.push_back(new Pin(Blend, In, 1));
    pins_.push_back(new Pin(CubeViewer, Out, 1));
}

void ColorAdjustNode::draw_node() {
     const float node_width = 100.0f;
    ImNodes::BeginNode(get_unique_id());

    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Color Adjust");
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(pins_[0]->unique_id_);
    ImGui::Text("Input Color");
    ImNodes::EndInputAttribute();

    ImGui::PushItemWidth(node_width);
    ImGui::DragFloat("Brightness", &brightness_, 0.01f, -1.0f, 1.0f);
    ImGui::DragFloat("Contrast", &contrast_, 0.01f, 0.0f, 2.0f);
    ImGui::DragFloat("Saturation", &saturation_, 0.01f, 0.0f, 2.0f);
    ImGui::PopItemWidth();

    ImNodes::BeginOutputAttribute(pins_[1]->unique_id_);
    ImGui::Text("Adjusted Color");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();

}

class CubeViewportNode : public Node {
public:
    explicit CubeViewportNode(glm::vec2 position = glm::vec2(0,0));
    void draw_node() override;
    void set_framebuffer(GLuint framebuffer, GLuint texturebuffer, GLuint texture) {
        framebuffer = framebuffer;
        textureBuffer = texturebuffer;
        render_texture_ = texture;
    }


private:
    GLuint textureBuffer;
    GLuint framebuffer;
    GLuint render_texture_;
};

CubeViewportNode::CubeViewportNode(glm::vec2 position) : Node(position, NodeType::CubeViewer),
    textureBuffer(0), render_texture_(0) {
    pins_.push_back(new Pin(ColorAdjust, In, 0));
    pins_.push_back(new Pin(Blend, In, 0));
}

void CubeViewportNode::draw_node() {

    ImNodes::BeginNode(get_unique_id());

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Cube Viewport");
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(get_unique_id());
    ImGui::TextUnformatted("Input");
    ImNodes::EndInputAttribute();

    ImGui::Text("Render View:");

    if (textureBuffer > 0) {
        ImGui::Image(reinterpret_cast<ImTextureID>(textureBuffer), ImVec2(100, 100));
    }
    ImNodes::EndNode();
}

