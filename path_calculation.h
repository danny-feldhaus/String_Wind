/*
Danny Feldhaus
path_calculation.h

    This file contains functions for calculating the string path instructions.
*/

#ifndef PATH_CALCULATION_H
#define PATH_CALCULATION_H
#include <cmath>
#include <vector>
#define cimg_use_png
#include <CImg.h>
#include <fstream>
#include <assert.h>
#include <string>
#include <bits/stdc++.h>

#if DEBUG
#include <iostream>
#include <experimental/filesystem>
#endif

#include "point.h"
#include "line_methods.h"
#include "image_methods.h"
using std::max;
using std::min;
using std::vector;
using cimg_library::CImg;
using string_wind::point;
using string_wind::line_methods;
using string_wind::image_methods;

namespace string_wind
{
    struct path_parameters
    {
        public:
            //The name of the input image's file
            char* file_name;
            //The size of the canvas in feet
            float canvas_size_in_feet = 1;
            //The width of the string in mm
            float string_width_in_mm = 0.127;
            //The locations of the pins that the strings are wound around
            std::vector<point<float>> pins;
            //The number of color channels (1 = gray, 3 = rgb)
            int channels;
            //The list of colored strings being used
            std::vector<unsigned char*> colors;
            float darkening_modifier;
            //The threshold at which the program will end and return the path
            float darkness_threshold;   

            //Constructor
            path_parameters(){};
            //Destructor
           // ~path_parameters();
            //Read the parameters from the given json file
            static path_parameters* read_from_json(const char* file_name);
    };


    class path_calculator
    {
        public:
            path_calculator(path_parameters* parameters);
            ~path_calculator();
            std::vector<int> calculate_path();
            cimg_library::CImg<unsigned char> draw_strings();
        private:            
            //The information for the image / path settings
            path_parameters* parameters;
            vector<point<int>> local_pins;
            //The path of the string, as a list of pin indices
            std::vector<int> path;
            //The original, unedited image
            cimg_library::CImg<unsigned char> input_image;
            cimg_library::CImg<unsigned char> mask;
            cimg_library::CImg<unsigned char> modified_image;
            //The grades for each path between pins. [p1][p2]
            float** path_grades;
            //The lengths for each path.
            int** path_lengths;
            //The points along each path.
            //std::vector<point<int>>** paths; 
            //The number of common pixels shared between each path
            cimg_library::CImg<unsigned char> shared_pixels;
            //The number of pins
            int pin_count;
            //The width of the string with the canvas of width 1.
            int unit_string_width;
            void calculate_initial_grades();
            void calculate_local_points(vector<point<float>>& unit_points);

    };
}
#endif

/*

    struct point<float>
    {
        float x;
        float y;

        point<float>()
        {
            x=0;
            y=0;
        }
        point<float>(const float _x, const float _y)
        {
            x = _x;
            y = _y;
        }
        int operator[](int i)
        {
            assert(i == 0 || i == 1);
            if(i==0)
            {
                return x;
            }
            return y;
        };
        bool operator==(const point<float> other)
        {
            return x==other.x && y==other.y;
        }
        bool operator!=(const point<float> other)
        {
            return x != other.x || y != other.y;
        }
        bool operator<(const point<float> other)
        {
            if( x < other.x)
            {
                return true;
            }
            if(x == other.x)
            {
                return y < other.y;
            }
            return false;
        }
        point<float>& operator = (const point<float>& other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }
        point<float>& operator * (const float mult)
        {
            x = x * mult;
            y = y * mult;
            return *this;
        }
        point<float>& operator / (const float div)
        {
            x = x / div;
            y = y / div;
            return *this;
        }

    };        
    
    inline std::ostream & operator <<(std::ostream & Str, point<float> & v)
    {
        Str << "(" << v.x << "," << v.y << ")";
        return Str;
    }
    struct point
    {
        int x;
        int y;

        point()
        {
            x=0;
            y=0;
        }
        point(const int _x, const int _y)
        {
            x = _x;
            y = _y;
        }
        int operator[](int i)
        {
            assert(i == 0 || i == 1);
            if(i==0)
            {
                return x;
            }
            return y;
        };
        bool operator==(const point other)
        {
            return x==other.x && y==other.y;
        }
        bool operator!=(const point other)
        {
            return x != other.x || y != other.y;
        }
        bool operator<(const point other)
        {
            if( x < other.x)
            {
                return true;
            }
            if(x == other.x)
            {
                return y < other.y;
            }
            return false;
        }
        point& operator = (const point& other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }
    };

    inline std::ostream & operator <<(std::ostream & Str, point & v)
    {
        Str << "(" << v.x << "," << v.y << ")";
        return Str;
    }
*/