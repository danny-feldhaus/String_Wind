
#ifndef COLOR_H
#define COLOR_H
#include <CImg.h>

using cimg_library::CImg;

namespace string_wind
{
    template <typename t>
    struct color_RGB
    {
        public:
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
            r = image(x,y,0,0);
            g = image(x,y,0,1);
            b = image(x,y,0,2);
        }
        t get_r() const
        {
            return r;
        }
        t get_g() const
        {
            return g;
        }
        t get_b() const
        {
            return b;
        }
        t* get_array()
        {
            return color_array;
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
        private:
            t r;
            t g;
            t b;
            t color_array[3];

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
        color_LAB(const color_RGB<t> rgb)
        {
            CImg<float> LAB_color_image_for_conversion = CImg<float>(1,1,1,3);
            //Convert the given RGB color to LAB by creating a 1 pixel image, converting that image, and then taking the new pixel's color.
            //This is a gross and roundabout way of doing it, but it guarantees that the image / color LAB equations are the same.
            LAB_color_image_for_conversion(0,0,0,0) = rgb.get_r();
            LAB_color_image_for_conversion(0,0,0,1) = rgb.get_g();
            LAB_color_image_for_conversion(0,0,0,2) = rgb.get_b();
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
        Str << "(" << v.get_r() << "," << v.get_g() << ", " << v.get_b() << ")";
        return Str;
    }
    inline std::ostream & operator <<(std::ostream & Str, color_RGB<int> & v)
    {
        Str << "(" << v.get_r() << "," << v.get_g() << ", " << v.get_b() << ")";
        return Str;
    }

}

#endif