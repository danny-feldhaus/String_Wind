#ifndef LINE_METHODS_H
#define LINE_METHODS_H
#include "point.h"
#include <CImg.h>
#include <vector>
#include <cmath>

using string_wind::point;
using cimg_library::CImg;
using std::vector;
using std::max;
using std::min;

namespace string_wind
{
    class line_methods
    {
        public:
            static float get_line_average_value(point<int> p0, point<int> p1,cimg_library::CImg<float>* image, cimg_library::CImg<float>* mask = nullptr);
            static bool do_intersect(point<int> & p1, point<int> &  q1, point<int> &  p2, point<int>  &  q2);
        private:
            static int orientation(point<int> &  p, point<int> &  q, point<int> &  r);
            static bool on_segment(point<int> &  p, point<int> &  q, point<int> &  r);
    };
}
#endif