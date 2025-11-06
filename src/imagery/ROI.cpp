#include "PchApp.h"

#include "ROI.h"

const char* ROI_NONE = "NONE";
const char* ROI_POINT = "POINT";
const char* ROI_RECT = "RECT";
const char* ROI_POLY = "POLY";


ROI::ROI () {
    active = true;
    colour_red = 128;
    colour_green = 128;
    colour_blue = 128;
}

ROI::~ROI () {
    entities.clear();
}


void ROI::set_name (const std::string& n) {
    name = n;
}

std::string ROI::get_name () {
    return name;
}


void ROI::set_color (int r, int g, int b) { set_colour(r,g,b); }
void ROI::set_colour (int r, int g, int b) {
    colour_red = r;
    colour_green = g;
    colour_blue = b;
}

void ROI::get_color (int* r, int* g, int* b) { get_colour(r,g,b); }
void ROI::get_colour (int* r, int* g, int* b) {
    *r = colour_red;
    *g = colour_green;
    *b = colour_blue;
}


void ROI::add_entity (ROIEntity* e) {
    entities.push_back(e);
}


std::vector<ROIEntity*> ROI::get_entities () {
    return entities;
}
