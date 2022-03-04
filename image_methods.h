#include <CImg.h>
#include <vector>
#include "point.h"
#include <cmath>
#if DEBUG
#include <iostream>
#endif
using cimg_library::CImg;
using std::vector;
using string_wind::point;
using std::min;
using std::max;
namespace string_wind
{
    class image_methods
    {
        public:
            static void Calculate_Mask(const CImg<float>& input_image, CImg<float>& output_mask);
            static void Convert_To_Grayscale(const CImg<float>& input_image, CImg<float>& output_image);
            static void Draw_Points(CImg<float>& input_image, const vector<point<int>> points, const float* color);
            static void Draw_Path(vector<int>& indices, vector<point<float>>& pins, CImg<float> & output_image);
            static point<int> Unit_To_Image(const point<float>& local_point, const CImg<float>& image);
            static point<float> Image_To_Unit(const point<int>& image_point, const CImg<float>& image);
            static CImg<float> Get_Color_Similarity(const CImg<float>& image, const float* color);
    };
}