#pragma once

struct Link {
    int id;
    int startPin;
    int endPin;

    Link(int linkId, int start, int end) : id(linkId), startPin(start), endPin(end) {}
};

