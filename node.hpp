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
    light,              // Základní světlo
    pointLight,         // Bodové světlo
    directionalLight,   // Směrové světlo
    spotLight,         // Bodové světlo s kuželem
    lightingModel,     // Node pro nastavení modelu osvětlení (Phong, Blinn-Phong)
    material           // Materiálové vlastnosti
};

struct Node
{
    NodeType type;
    float    value;

    explicit Node(const NodeType t) : type(t), value(0.f) {}
    Node(const NodeType t, const float v) : type(t), value(v) {}
};
