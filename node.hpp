#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>

enum class NodeType
{
    add,
    multiply,
    output,
    sine,
    time,
    value,
    power
};

struct Node
{
    NodeType type;
    float    value;

    explicit Node(const NodeType t) : type(t), value(0.f) {}

    Node(const NodeType t, const float v) : type(t), value(v) {}


    /*void compute(Node &node) {
        //for (auto& node : nodes) {
        if (node.type == NodeType::Operation) {
            if (node.name == "Add") {
                float a = node.parameters["Input A"];
                float b = node.parameters["Input B"];
                node.computedOutputs["Result"] = a + b;
            }
        }
        //}
    }*/
};
/*
inline std::string NodeTypeToString(NodeType type)
{
    switch (type)
    {
    case NodeType::Parameter:   return "Parametr";
    case NodeType::Operation:   return "Operation";
    case NodeType::Output:      return "Output";
    case NodeType::Texture:     return "Texture";

    default:      return "[Unknown type]";
    }
}*/
