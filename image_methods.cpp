#include "image_methods.h"
using string_wind::image_methods;

void image_methods::Calculate_Mask(const CImg<unsigned char>& input_image, CImg<unsigned char>& output_mask)
{
    output_mask = CImg<unsigned char>(input_image.width(),input_image.height(), 1, 1,0); 
    CImg<unsigned char>   imgR(input_image.width(), input_image.height(), 1, 3, 0),
                                        imgG(input_image.width(), input_image.height(), 1, 3, 0),
                                        imgB(input_image.width(), input_image.height(), 1, 3, 0);
    cimg_forXY(input_image,x,y)
    {
        #if DEBUG
        //if(x * y == 0) std::cout << "Checking " << x << ", " << y << ", ImageS Size: " << output_mask.size() << '\n';
        #endif
        // Separation of channels
        int R = (int)input_image(x,y,0,0);
        int G = (int)input_image(x,y,0,1);
        int B = (int)input_image(x,y,0,2);
        // mask is false when the image is completely black
        int mask_val = (int)(R + G + B > 0);
        // saving píxel values into image information
        output_mask(x,y,0,0) = (unsigned char)mask_val;
    }
}

void image_methods::Convert_To_Grayscale(const CImg<unsigned char>& input_image, CImg<unsigned char>& output_image)
{
    CImg<unsigned char> gray = CImg<unsigned char>(input_image.width(),input_image.height(), 1, 1,0);
    CImg<unsigned char>   imgR(input_image.width(), input_image.height(), 1, 3, 0),
                                        imgG(input_image.width(), input_image.height(), 1, 3, 0),
                                        imgB(input_image.width(), input_image.height(), 1, 3, 0);
    cimg_forXY(input_image,x,y)
    {
        // Separation of channels
        int R = (int)input_image(x,y,0,0);
        int G = (int)input_image(x,y,0,1);
        int B = (int)input_image(x,y,0,2);
        // Real weighted addition of channels for gray
        int grayValueWeight = (int)(0.299*R + 0.587*G + 0.114*B);
        // saving píxel values into image information
        gray(x,y,0,0) = (unsigned char)grayValueWeight;
    }
    output_image = gray;
}


void image_methods::Draw_Points(CImg<unsigned char>& input_image, const vector<point<int>> points, const unsigned char* color)
{
    int pixel_width = input_image.spectrum();
    int min_size = min(input_image.width(),input_image.height());
    for(point<int> p : points)
    {
        for(int i=0; i<pixel_width; i++)
        {
            input_image(min_size * p.x,min_size * p.y,0,i) = color[i];
        }
    }
}

void image_methods::Draw_Path(vector<int>& indices, vector<point<float>>& pins, CImg<unsigned char> & output_image)
{
    int indexA, indexB;
    int image_width = output_image.width();
    int image_height = output_image.height();
    int min_size = min(image_height,image_width);
    point<int> pointA, pointB;
    const unsigned char color[] = {0};
    #if DEBUG
    std::cout << "Drawing path of length " << (int)indices.size() << "\n";
    #endif
    for(int i=0; i < (int)indices.size()-1; i++)
    {
        indexA = indices[i];
        indexB = indices[i+1];
        pointA = point<int>(min_size * pins[indexA].x,min_size * pins[indexA].y);
        pointB = point<int>(min_size * pins[indexB].x, min_size * pins[indexB].y);
        output_image.draw_line(pointA.x,pointA.y,pointB.x,pointB.y, color);
    }
}