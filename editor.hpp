#pragma once

#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_set>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "node.hpp"
#include "link.hpp"
#include "graph.hpp"
#include "object.hpp"
#include "shader.hpp"
#include "framebuffer.hpp"

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
        frameBUfferSphere.InitFrameBuffer(800, 600);
        mainShader.loadShader(shaderVertex, TypeShader::VERTEX_SHADER);
        mainShader.loadShader(shaderFragment, TypeShader::FRAGMENT_SHADER);
        mainShader.createShaderProgram();

        // first viewer is cube.
        frameBuffer.Bind();
        render_to_framebuffer_cube(glm::vec3(1.0f, 0.5f, 0.31f));
        frameBuffer.Unbind();
    }

private:
    unsigned int fbo;      // Framebuffer Object
    unsigned int texture;
    unsigned int rbo, ebo;
    unsigned int cubeVAO, cubeVBO = 0;
    unsigned int sphereVAO, sphereVBO = 0;
    Shader mainShader;
    FrameBuffer frameBuffer;
    FrameBuffer frameBUfferSphere;

    int m_latitudeSegments = 0;
    int m_longitudeSegments = 0;

public:

    uint32_t getTicks() {
        static const auto startTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        return static_cast<uint32_t>(elapsed);
    }


    void renderCube(const glm::vec3& color, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) {
        // if(cubeVAO == 0) {
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndices), CubeIndices, GL_STATIC_DRAW);

        glBindVertexArray(cubeVAO);


        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);


        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);


        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        // }


        mainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "model"), 1, GL_FALSE, &model[0][0]);
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), "color"), 1, &color[0]);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void setupSphere(float radius, int latitudeSegments, int longitudeSegments) {
        auto vertices = generateSphere(radius, latitudeSegments, longitudeSegments);

        m_latitudeSegments = latitudeSegments;
        m_longitudeSegments = longitudeSegments;

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);

        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);                     // Pozice
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));  // Normály
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));  // UV
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);


    }

    void renderSphere(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model, const glm::vec3& color) {
        mainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainShader.getShaderProgram(), "model"), 1, GL_FALSE, &model[0][0]);
        glUniform3fv(glGetUniformLocation(mainShader.getShaderProgram(), "color"), 1, &color[0]);

        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, (m_latitudeSegments + 1) * (m_longitudeSegments + 1));
        glBindVertexArray(0);
    }

    void render_to_framebuffer_cube(glm::vec3 color)
    {
        glViewport(0, 0, 800, 600);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model = glm::mat4(1.0f);
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

    void show()
    {
        // Update timer context
        current_time_seconds = 0.001f *  getTicks();

        auto flags = ImGuiWindowFlags_MenuBar;

        // The node editor window
        ImGui::Begin("color node editor", NULL, flags);

        if (ImGui::BeginMenuBar())
        {
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

                    graph_.insert_edge(ui_node.id, ui_node.ui.add.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.ui.add.rhs);

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
                    // graph_.insert_edge(ui_node.id, ui_node.ui.viewport.input);
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
                    // graph_.insert_edge(ui_node.id, ui_node.ui.viewport.input);
                    nodes_.push_back(ui_node);
                    std::cout << "Sphere viewport node created with ID: " << ui_node.id << std::endl;
                    ImNodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
        }

        for (const UiNode& node : nodes_)
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
                }

                frameBuffer.Bind();
                render_to_framebuffer_cube(color);
                //frameBuffer.RescaleFrameBuffer(50,50);
                ImGui::Image((ImTextureID)frameBuffer.getFrameTexture(), ImVec2(200, 200));
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
                ImGui::Image((ImTextureID)frameBUfferSphere.getFrameTexture(), ImVec2(200, 200));
                frameBUfferSphere.Unbind();
                ImNodes::EndNode();
            }
            break;
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
                    //case UiNodeType::viewport:
                    //    graph_.erase_node(iter->ui.viewport.input);
                    //    break;
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

        //ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
        ImGui::Begin("3d view");
        ImVec2 size = ImVec2(200, 200);
        //frameBuffer.RescaleFrameBuffer(300,300);
        ImGui::Image(ImTextureID(frameBuffer.getFrameTexture()), size);
        ImGui::End();
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
        sphereviewport
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

        } ui;
    };

    Graph<Node>            graph_;
    std::vector<UiNode>    nodes_;
    int                    root_node_id_;
    ImNodesMiniMapLocation minimap_location_;
    bool showSphere;
};
