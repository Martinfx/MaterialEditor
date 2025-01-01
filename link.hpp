#pragma once

#include "node.hpp"

struct Link {
    Link() {}
    Link(int input_pin, int output_pin, Node* input_node, Node* output_node);
    ~Link() { }

    int id_;
    int input_slot_id_ = -1;
    int output_slot_id_ = -1;
    Node* input_node_;
    Node* output_node_;

    void transfer_data();
};

static int unique_link_id_ = -1;

const int get_available_link_id() {
    unique_link_id_++;
    return unique_link_id_;
}

Link::Link(int input_pin, int output_pin, Node* input_node, Node* output_node)
    : input_slot_id_(input_pin), output_slot_id_(output_pin),
    input_node_(input_node), output_node_(output_node) {
    id_ = get_available_link_id();
}
