#include <stdio.h>
#include <cmath>
#include <string>
#include "path_calculation.h"
#include "color.h"

using string_wind::path_calculator;
using string_wind::path_parameters;
using string_wind::point;
using string_wind::point;
using string_wind::color_RGB;
using std::vector;
using cimg_library::CImg;


vector<point<float>> Circular_Pins(const int pin_count, const float radius, const float center_x, const float center_y)
{
    vector<point<float>> pins;
    float angle;
    float xpos, ypos;
    for(int i=0; i<pin_count; i++)
    {
        angle = 2 * 3.14159 * (float)i/pin_count;
        xpos = cos(angle)*radius + center_x;
        ypos = sin(angle)*radius + center_y;
        pins.push_back(point<float>(xpos,ypos));
    }
    return pins;
}
int main(int argc, char** args)
{
    path_parameters parameters;
    int pin_count;
    for(int i=0; i<argc; i++)
    {
        if(strncmp(args[i],(char*)"-P=",3)==0)
        {
            pin_count = atoi(args[i]+3);
        }
        else if(strncmp(args[i],(char*)"-F=",3)==0)
        {
            parameters.file_name = args[i]+3;
        }
        else if(strncmp(args[i],(char*)"-D=",3)==0)
        {
            parameters.darkness_threshold = atoi(args[i]+3);
        }
        else if(strncmp(args[i],(char*)"-M=",3)==0)
        {
            parameters.darkening_modifier = atof(args[i]+3);
        }
        else if(strncmp(args[i],(char*)"-C=",3)==0)
        {
            parameters.canvas_size_in_feet = atof(args[i]+3);
        }
        else if(strncmp(args[i],(char*)"-S",3)==0)
        {
            parameters.string_width_in_mm = atof(args[i]+3);
        }
        else if(strncmp(args[i],(char*)"-c",3)==0)
        {
            
        }
    }

    parameters.pins = Circular_Pins(pin_count,0.49,0.5,0.5);
    /*
    float black[] = {0,0,0};
    float white[] = {255.0,255.0,255.0};
    //float red[] = {255.0,0,0};
    //float yellow[] = {255,255,0};
    //float green[] = {0,255.0,0};
    //float blue[] = {0,0,255.0};

    float skin[] = {214,119,117};
    float brown[] = {103,73,56};
    float beard[] = {224,188,164};
*/

    //parameters.colors.push_back(color_RGB<float>(88, 78, 66 ));
    //parameters.colors.push_back(color_RGB<float>(229,229,217));
    //parameters.colors.push_back(color_RGB<float>(168,181,164));
    //parameters.colors.push_back(color_RGB<float>(153,162,166));
    parameters.colors.push_back(color_RGB<float>(0,0,0));
    //parameters.colors.push_back(color_RGB<float>(255,255,255));

    parameters.darkening_modifier = 0.5;
    parameters.darkness_threshold = 40;
    #if DEBUG
    std::cout << "Creating Calculator...";
    #endif 
    path_calculator calculator = path_calculator(&parameters); 
    #if DEBUG
    std::cout << "Done.\n";
    std::cout << "Calculating path...";
    #endif
    calculator.calculate_path();
    #if DEBUG
    std::cout << "Drawing path...";
    #endif
    CImg<float> string_image = calculator.draw_strings();
    #if DEBUG
    std::cout << "Done.\n";
    std::cout << "Saving image... ";
    #endif
    string_image.save("Output_Images/final_image.png");
    #if DEBUG
    std::cout << "Done.\n";
    std::cout << "Finished.\n";
    #endif
    return 0;
}