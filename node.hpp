#pragma once

enum class NodeType
{
    add,
    multiply,
    output,
    sine,
    time,
    value,
    power,
    cubeviewport,
    spherevieport,
    texture,
    blend,
    colorAdjust,
    filter,
    light,
    pointLight,
    directionalLight,
    spotLight,
    lightingModel,     //  (Phong, Blinn-Phong)
    material
};

struct Node
{
    NodeType type;
    float    value;

    explicit Node(const NodeType t) : type(t), value(0.f) {}
    Node(const NodeType t, const float v) : type(t), value(v) {}
};
