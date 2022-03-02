#include <stdio.h>
#include <cmath>
#include "path_calculation.h"

using string_wind::path_calculator;
using string_wind::path_parameters;
using string_wind::point;
using string_wind::fpoint;
using std::vector;
using cimg_library::CImg;

vector<fpoint> Circular_Pins(const int pin_count, const float radius, const float center_x, const float center_y)
{
    vector<fpoint> pins;
    float angle;
    float xpos, ypos;
    for(int i=0; i<pin_count; i++)
    {
        angle = 2 * 3.14159 * (float)i/pin_count;
        xpos = cos(angle)*radius + center_x;
        ypos = sin(angle)*radius + center_y;
        pins.push_back(fpoint(xpos,ypos));
    }
    return pins;
}
int main()
{
    int pin_count = 256;
    path_parameters parameters;
    parameters.file_name = (char*)"daddy3.png";
    parameters.canvas_size_in_feet = 2;
    parameters.pins = Circular_Pins(pin_count,0.49,0.5,0.5);
    parameters.channels = 1;
    unsigned char black = 0;
    parameters.colors.push_back(&black);
    parameters.darkening_modifier = 0.5;
    parameters.darkness_threshold = 40;

    path_calculator calculator = path_calculator(&parameters);
    vector<int> path = calculator.calculate_path();
    CImg<unsigned char> string_image = calculator.draw_strings();
    string_image.save("final_string_image.png");
    return 0;
}