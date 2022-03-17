#include <CImg.h>
#include <vector>
#include "point.h"
#include <cmath>
#if DEBUG
#include <iostream>
using std::cout;
#endif

using cimg_library::CImg;
using std::vector;
using string_wind::point;
using std::min;
using std::max;
namespace string_wind
{
    template <typename t>
    struct color_RGB
    {
        t r;
        t g;
        t b;
        t color_array[3];
        color_RGB()
        {
            r = 0;
            g = 0;
            b = 0;
        }
        color_RGB(const t _r,const t _g,const t _b)
        {
            r = _r;
            g = _g;
            b = _b;
            color_array[0] = r;
            color_array[1] = g;
            color_array[2] = b;
        }
        void set_values(const t _r, const t _g, const t _b)
        {
            r = _r;
            g = _g;
            b = _b;
            color_array[0] = r;
            color_array[1] = g;
            color_array[2] = b;
        }
        void set_from_array(const t* _color_array)
        {
            r = _color_array[0];
            g = _color_array[1];
            b = _color_array[2];
            color_array[0] = _color_array[0];
            color_array[1] = _color_array[1];
            color_array[2] = _color_array[2];
        }
        void set_from_image(const CImg<float>& image, const int x, const int y)
        {
            r = 
        }
        float get_grayscale()
        {
            return 0.299*r + 0.587*g + 0.114*b;
        }
        const color_RGB<t>& operator =(const color_RGB<t>& other)
        {
            r = other.r;
            g = other.g;
            b = other.b;
            return *this;
        }
        const bool operator ==(const color_RGB<t>& other)
        {
            return r == other.r && g == other.g && b == other.b;
        }
        const bool operator !=(const color_RGB<t>& other)
        {
            return (r != other.r) ||(g != other.g) || (b != other.b);
        }
        friend const bool operator <(const color_RGB<t>& a, const color_RGB<t>& b)
        {
            if(a.r < b.r)
            {
                return true;
            }
            if(a.r == b.r)
            {
                if(a.g < b.g)
                {
                    return true;
                }
                if(a.g == b.g)
                {
                    return a.b < b.b;
                }
            }
            return false;
        }
    };

    template <typename t>
    struct color_LAB
    {
        t L;
        t A;
        t B;
        color_LAB()
        {
            L=0;
            A=0;
            B=0;
        }
        color_LAB(const t _L, const t _A, const t _B)
        {
            L = _L;
            A = _A;
            B = _B;
        }
        color_LAB(const color_RGB<t> color_RGB)
        {
            CImg<float> LAB_color_image_for_conversion = CImg<float>(1,1,1,3);
            //Convert the given RGB color to LAB by creating a 1 pixel image, converting that image, and then taking the new pixel's color.
            //This is a gross and roundabout way of doing it, but it guarantees that the image / color LAB equations are the same.
            LAB_color_image_for_conversion(0,0,0,0) = color_RGB.r;
            LAB_color_image_for_conversion(0,0,0,1) = color_RGB.g;
            LAB_color_image_for_conversion(0,0,0,2) = color_RGB.b;
            LAB_color_image_for_conversion.RGBtoLab();
            set_from_array(LAB_color_image_for_conversion.data(0,0,0));
        } 
        void set_from_array(const t* color_array)
        {
            L = color_array[0];
            A = color_array[1];
            B = color_array[2];
        }
    };

    inline std::ostream & operator <<(std::ostream & Str, color_RGB<float> & v)
    {
        Str << "(" << v.r << "," << v.g << ", " << v.b << ")";
        return Str;
    }
    inline std::ostream & operator <<(std::ostream & Str, color_RGB<int> & v)
    {
        Str << "(" << v.r << "," << v.g << ", " << v.b << ")";
        return Str;
    }

    class image_methods
    {
        public:
            static void Calculate_Mask(const CImg<float>& input_image, CImg<float>& output_mask);
            static void Convert_To_Grayscale(const CImg<float>& input_image, CImg<float>& output_image);
            static void Draw_Points(CImg<float>& input_image, const vector<point<int>> points, color_RGB<float> color);
            static void Draw_Path(vector<int>& indices, vector<point<float>>& pins, CImg<float> & output_image, const float* color);
            static point<int> Unit_To_Image(const point<float>& local_point, const CImg<float>& image);
            static point<float> Image_To_Unit(const point<int>& image_point, const CImg<float>& image);
            static CImg<float> Get_Color_Similarity(const CImg<float>& image, const color_RGB<float> color);
            static vector<color_RGB<float>> Get_Color_Palette(const CImg<float>& image, const int pallet_size, const float histogram_cell_size = 16.0);
    };
}