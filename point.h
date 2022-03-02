#ifndef POINT_H
#define POINT_H
#include <type_traits>
#include <ostream>
#include <cassert>
namespace string_wind
{
    template <typename t>
    struct point
    {
        t x;
        t y;
        point()
        {
            assert(std::is_arithmetic<t>());
            x = 0;
            y = 0;
        }
        point(const t _x,const t _y)
        {
            assert(std::is_arithmetic<t>());
            x = _x;
            y = _y;
        }
        const bool operator ==(const point<t>& other)
        {
            return x == other.x && y == other.y;
        }
        const bool operator !=(const point<t>& other)
        {
            return !(&this == other);
        }
        const bool operator < (const point<t>& other)
        {
            if(x < other.x)
            {
                return true;
            }
            if(x == other.x)
            {
                return y < other.y;
            }
            return false;
        }
        const bool operator <= (const point<t>& other)
        {
            return (&this < other) || (this == other);
        }
        const bool operator > (const point<t>& other)
        {
            return !(&this <= other);
        }
        const bool operator >=(const point<t>& other)
        {
            return (&this == other) || (&this > other);
        }
        point<t>& operator * (const t mult)
        {
            return point(x * mult, y * mult);
        }
        point<t>& operator / (const t mult)
        {
            return point(x / mult, y / mult);
        }
        point<t>& operator + (const point<t>& other)
        {
            return point(x + other.x, y + other.y);
        }
        point<t>& operator - (const point<t>& other)
        {
            return point(x - other.x, y - other.y);
        }
        void operator = (const point<t>& other)
        {
            x = other.x;
            y = other.y;
        }
    };

    inline std::ostream & operator <<(std::ostream & Str, point<int> & v)
    {
        Str << "(" << v.x << "," << v.y << ")";
        return Str;
    }
    inline std::ostream & operator <<(std::ostream & Str, point<float> & v)
    {
        Str << "(" << v.x << "," << v.y << ")";
        return Str;
    }
}
#endif