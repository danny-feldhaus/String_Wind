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
#include <bits/stdc++.h>
#include <algorithm>

#if DEBUG
#include <iostream>
#include <experimental/filesystem>
#include <string>
//#include <format>
#endif

#include "point.h"
#include "line_methods.h"
#include "image_methods.h"
#include "color.h"

using std::max;
using std::min;
using std::vector;
using cimg_library::CImg;
using string_wind::point;
using string_wind::line_methods;
using string_wind::image_methods;
using string_wind::color_RGB;

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
            std::vector<color_RGB<float>> colors;
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

    struct pin_and_grade
    {
        int pin;
        float grade;
        pin_and_grade()
        {
            pin = -1;
            grade = -1.0;
        }
        pin_and_grade(int _pin,float _grade)
        {
            pin = _pin;
            grade = _grade;
        }
    };

    struct path_step
    {
        int from_pin;
        int to_pin;
        color_RGB<float> color;
        path_step(int _from_pin, int _to_pin, color_RGB<float> _color)
        {
            from_pin = _from_pin;
            to_pin = _to_pin;
            color = _color;
        }
        const bool operator==(path_step& other)
        {
            return from_pin == other.from_pin && to_pin == other.to_pin && color == other.color;
        }
    };

    class path_instance{
        public:
            path_instance(CImg<float>* _values, path_parameters* _parameters, vector<point<int>>* _local_points, CImg<float>* _mask);
            //Find the next pin in the path, and return its index.
            int move_to_next();
            pin_and_grade get_next_best_pin_and_score();
            int get_cur_pin();
            vector<int>& get_path();
        private:
            CImg<float>* values;
            CImg<float>* mask;
            path_parameters* parameters;
            vector<point<int>>* local_points;
            vector<int> path;
            int pin_count;

            CImg<float> path_grades;
            CImg<float> path_lengths;

            float cur_best_score = 0;

            void calculate_initial_grades();
    };

    class path_calculator
    {
        public:
            path_calculator(path_parameters* parameters);
            ~path_calculator();
            void calculate_path();
            cimg_library::CImg<float> draw_strings();
            static void calculate_local_points(const vector<point<float>>& unit_points,const CImg<float>& image, vector<point<int>>& output_points);

        private:            
            //The information for the image / path settings
            path_parameters* parameters;
            vector<point<int>> local_pins;
            vector<CImg<float>> color_difference_maps;
            vector<path_instance> path_instances;
            vector<path_step> path;
            CImg<float> full_image;
            CImg<float> mask;

            int unit_string_width;
            void calculate_initial_grades();
            void load_from_parameters();
    };

}
#endif