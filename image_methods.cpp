#include "image_methods.h"
using string_wind::image_methods;

void image_methods::Calculate_Mask(const CImg<float>& input_image, CImg<float>& output_mask)
{
    output_mask = CImg<float>(input_image.width(),input_image.height(), 1, 1,0); 
    CImg<float> imgR(input_image.width(), input_image.height(), 1, 3, 0),
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
        output_mask(x,y,0,0) = (float)mask_val;
    }
}

void image_methods::Convert_To_Grayscale(const CImg<float>& input_image, CImg<float>& output_image)
{
    CImg<float> gray = CImg<float>(input_image.width(),input_image.height(), 1, 1,0);
    CImg<float>   imgR(input_image.width(), input_image.height(), 1, 3, 0),
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
        gray(x,y,0,0) = (float)grayValueWeight;
    }
    output_image = gray;
}


void image_methods::Draw_Points(CImg<float>& input_image, const vector<point<int>> points, const float* color)
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

void image_methods::Draw_Path(vector<int>& indices, vector<point<float>>& pins, CImg<float> & output_image)
{
    int indexA, indexB;
    int image_width = output_image.width();
    int image_height = output_image.height();
    int min_size = min(image_height,image_width);
    point<int> pointA, pointB;
    const float color[] = {0};
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

point<int> image_methods::Unit_To_Image(const point<float>& local_point, const CImg<float>& image)
{
    int max_dimension = max(image.height(),image.width());
    return point<int>(max_dimension * local_point.x, max_dimension * local_point.y);
}

point<float> image_methods::Image_To_Unit(const point<int>& image_point, const CImg<float>& image)
{
    int max_dimension = max(image.height(),image.width());
    float x = static_cast<float>(image_point.x) / max_dimension;
    float y = static_cast<float>(image_point.y) / max_dimension;
    return point<float>(x,y);
}

CImg<float>image_methods::Get_Color_Similarity(const CImg<float>& image, const float* color)
{
    //A roundabout way to convert to LAB. 
    //TODO: Write a specific rgb2lab function for individual colors.
    CImg<float> LAB_color_image_for_conversion = CImg<float>(1,1,1,3);
    CImg<float> L_channel, A_channel, B_channel, difference_image;
    //Convert the given RGB color to LAB by creating a 1 pixel image, converting that image, and then taking the new pixel's color.
    //This is a gross and roundabout way of doing it, but it guarantees that the image / color LAB equations are the same.
    LAB_color_image_for_conversion(0,0,0,0) = color[0];
    LAB_color_image_for_conversion(0,0,0,1) = color[1];
    LAB_color_image_for_conversion(0,0,0,2) = color[2];
    LAB_color_image_for_conversion.RGBtoLab();
    float* LAB_color = LAB_color_image_for_conversion.data();
    #if DEBUG
    std::cout << "L: " << LAB_color[0] << ", A: " << LAB_color[1] << ", B: " << LAB_color[2] << '\n';
    #endif
    CImg<float>LAB_image = CImg<float>((image.get_shared_channels(0,2)).get_RGBtoLab());

    //Calculate the deltaE by subtracting the color from the image, summing the square of the channels, and taking the square root. 
    //LAB_image -= LAB_color;
    L_channel = LAB_image.get_shared_channels(0,0);
    L_channel = (L_channel - LAB_color[0]).sqr();
    A_channel = LAB_image.get_shared_channels(1,1);
    A_channel = (A_channel - LAB_color[1]).sqr();
    B_channel = LAB_image.get_shared_channels(2,2);
    B_channel = (B_channel - LAB_color[2]).sqr();
    difference_image = (L_channel + A_channel + B_channel).sqrt();

    //Return the negative of the image, so that higher number = more similar.
    return (255 - CImg<float>(difference_image.get_normalize(0,255)));
}
