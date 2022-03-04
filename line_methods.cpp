#include "line_methods.h"
using string_wind::line_methods;

float line_methods::get_line_average_value(point<int> p0, point<int> p1,CImg<float>*image, CImg<float>* mask)
{
    point<int> cur_pixel;
    vector<point<int>> line_points;

    int dx,dy,dx1,dy1,px,py,xe,ye,i;
    int sum = 0;
    float length = 0;

    dx=p1.x - p0.x;
    dy=p1.y - p0.y;
    dx1=fabs(dx);
    dy1=fabs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;
    if(dy1<=dx1)
    {
        if(dx>=0)
        {
            cur_pixel.x=p0.x;
            cur_pixel.y=p0.y;
            xe=p1.x;
        }
        else
        {
            cur_pixel.x=p1.x;
            cur_pixel.y=p1.y;
            xe= p0.x;
        }

        for(i=0;cur_pixel.x < xe-1;i++)
        {
            cur_pixel.x++;
            if(px < 0)
            {
                px=px+2*dy1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    cur_pixel.y++;
                }
                else
                {
                    cur_pixel.y--;
                }
                px=px+2*(dy1-dx1);
            }
            if(!mask || ((*mask)(cur_pixel.x,cur_pixel.y,0,0)))
            {
                sum += (int)(*image)(cur_pixel.x,cur_pixel.y,0,0);
                length++;
                //line_points.push_back(point<int>(cur_pixel.x, cur_pixel.y));
            }
        }
    }
    else
    {
        if(dy>=0)
        {
            cur_pixel.x=p0.x;
            cur_pixel.y=p0.y;
            ye=p1.y;
        }
        else
        {
            cur_pixel.x = p1.x;
            cur_pixel.y = p1.y;
            ye = p0.y;
        }
        //if(!mask || ((*mask)(cur_pixel.x,cur_pixel.y,0,0)))
        //{
        //    line_points.push_back(point<int>(cur_pixel.x, cur_pixel.y));
        //}
        for(i=0; cur_pixel.y < ye-1; i++)
        {
            cur_pixel.y++;
            if(py<=0)
            {
                py=py+2*dx1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    cur_pixel.x++;
                }
                else
                {
                    cur_pixel.x--;
                }
                py=py+2*(dx1-dy1);
            }
            if(!mask || ((*mask)(cur_pixel.x,cur_pixel.y,0,0)))
            {
                sum += (int)(*image)(cur_pixel.x,cur_pixel.y,0,0);
                length++;
                //line_points.push_back(point<int>(cur_pixel.x, cur_pixel.y));
            }        
        }
    }
    return (float)sum / length;
}



bool line_methods::on_segment(point<int>& p1, point<int>& p2, point<int>& p3) {   //check whether p is on the line or not
   if(p3.x <= max(p1.x, p2.x) && p3.x <= min(p1.x, p2.x) &&
      (p3.y <= max(p1.y, p2.y) && p3.y <= min(p1.y, p2.y)))
      return true;
   
   return false;
}

int line_methods::orientation(point<int>& a, point<int>& b, point<int>& c) {
   int val = (b.y-a.y)*(c.x-b.x)-(b.x-a.x)*(c.y-b.y);
   if (val == 0)
      return 0;     //colinear
   else if(val < 0)
      return 2;    //anti-clockwise direction
    return 1;    //clockwise direction
}

bool line_methods::do_intersect(point<int>& PA0, point<int>& PA1, point<int>& PB0, point<int>& PB1) {
   //four direction for two lines and points of other line
   int dir1 = orientation(PA0, PA1, PB0);
   int dir2 = orientation(PA0, PA1, PB1);
   int dir3 = orientation(PB0, PB1, PA0);
   int dir4 = orientation(PB0, PB1, PA1);
    if((PA0 == PB0 && PA1 == PB1) || (PA0 == PB1 && PA1 == PB0))
        return true;

   if(dir1 != dir2 && dir3 != dir4)
      return true; //they are intersecting

   if(dir1==0 && on_segment(PA0,PA1,PB0)) //when p2 of line2 are on the line1
      return true;

   if(dir2==0 && on_segment(PA0,PA1, PB1)) //when p1 of line2 are on the line1
      return true;

   if(dir3==0 && on_segment(PB0,PB1, PA0)) //when p2 of line1 are on the line2
      return true;

   if(dir4==0 && on_segment(PB0,PB1, PA1)) //when p1 of line1 are on the line2
      return true;
         
   return false;
}