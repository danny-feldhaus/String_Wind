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
            static void Calculate_Mask(const CImg<unsigned char>& input_image, CImg<unsigned char>& output_mask);
            static void Convert_To_Grayscale(const CImg<unsigned char>& input_image, CImg<unsigned char>& output_image);
            static void Draw_Points(CImg<unsigned char>& input_image, const vector<point<int>> points, const unsigned char* color);
            static void Draw_Path(vector<int>& indices, vector<point<float>>& pins, CImg<unsigned char> & output_image);
            static point<int> Local_To_Image(const point<float>& local_point,const CImg<unsigned char>& image);
            //static point<float> Image_To_Local(const point<int>& image_point,const CImg<unsigned char>& image);
            static point<float> Image_To_Local(const point<int>& image_point, const CImg<unsigned char>& image);
    };
}