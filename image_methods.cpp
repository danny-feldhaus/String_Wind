#include "image_methods.h"
#include <map>
using string_wind::image_methods;
using string_wind::color_RGB;
using string_wind::color_LAB;


bool sort_by_count(const std::pair<color_RGB<float>, int> a, const std::pair<color_RGB<float>, int> b)
{
    return a.second > b.second;
} 
vector<color_RGB<float>> image_methods::Get_Color_Palette(const CImg<float>& image, const int palette_size, const float histogram_cell_size)
{
    color_RGB<float> cur_color;
    color_RGB<int> cur_cell;
    color_RGB<float> average_cell_color;
    std::map<color_RGB<int>, int> histogram_cells;
    vector<std::pair<color_RGB<int>,int>> pairs;
    vector<std::pair<color_RGB<int>,int>> best_pairs;
    vector<color_RGB<float>> palette;

    cimg_forXY(image, x, y)
    {
        #if DEBUG
        //cout << "Raw Data: " << image(x,y,0,0) << ", " << image(x,y,0,1) << ", " << image(x,y,0,2) << "\n";

        #endif
        cur_color.set_from_image(image,x,y);
        #if DEBUG
        //std::cout << "Pre-Cell Floor: " << cur_color << '\n';
        #endif
        
        cur_cell.set_values((int)(cur_color.get_r() / histogram_cell_size),
                            (int)(cur_color.get_g() / histogram_cell_size),
                            (int)(cur_color.get_b() / histogram_cell_size));
        histogram_cells[cur_cell]++;
        //std::cout << "Color " << cur_cell << " has " << histogram_cells[cur_cell] << " occurrences.\n";
    }
    for(auto& it : histogram_cells)
    {
        pairs.push_back(it);
    }
    
    std::sort(pairs.begin(), pairs.end(), [](auto &left, auto& right)
    {
        return left.second > right.second;
    });

    for(int i=0; i<palette_size; i++)
    {
        best_pairs.push_back(pairs[i]);
    }

    for(int i=0; i<palette_size; i++)
    {   
        average_cell_color.set_values(histogram_cell_size/2 + best_pairs[i].first.get_r() * histogram_cell_size,
                                      histogram_cell_size/2 + best_pairs[i].first.get_g() * histogram_cell_size,
                                      histogram_cell_size/2 + best_pairs[i].first.get_b() * histogram_cell_size);
        #if DEBUG
        std::cout << "Added color " << average_cell_color << " with count " << best_pairs[i].second << '\n';
        #endif
        
        palette.push_back(color_RGB<float>(average_cell_color.get_r(), average_cell_color.get_g(), average_cell_color.get_b()));
    }
    return palette;
}

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

void image_methods::Draw_Path(vector<int>& indices, vector<point<float>>& pins, CImg<float> & output_image, const float* color)
{
    int indexA, indexB;
    int image_width = output_image.width();
    int image_height = output_image.height();
    int min_size = min(image_height,image_width);
    point<int> pointA, pointB;
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
    int max_dimension = min(image.height(),image.width());
    return point<int>(max_dimension * local_point.x, max_dimension * local_point.y);
}

point<float> image_methods::Image_To_Unit(const point<int>& image_point, const CImg<float>& image)
{
    int max_dimension = max(image.height(),image.width());
    float x = static_cast<float>(image_point.x) / max_dimension;
    float y = static_cast<float>(image_point.y) / max_dimension;
    return point<float>(x,y);
}

CImg<float>image_methods::Get_Color_Similarity(const CImg<float>& image, color_RGB<float> color)
{
    //A roundabout way to convert to LAB. 
    //TODO: Write a specific rgb2lab function for individual colors.
    CImg<float> L_channel, A_channel, B_channel, difference_image;
    color_LAB<float> LAB_color = color_LAB<float>(color);
    #if DEBUG
    std::cout << "L: " << LAB_color.L << ", A: " << LAB_color.A << ", B: " << LAB_color.B << '\n';
    #endif
    CImg<float>LAB_image = CImg<float>((image.get_shared_channels(0,2)).get_RGBtoLab());

    //Calculate the deltaE by subtracting the color from the image, summing the square of the channels, and taking the square root. 
    //LAB_image -= LAB_color;
    L_channel = LAB_image.get_shared_channels(0,0);
    L_channel = (L_channel - LAB_color.L).sqr();
    A_channel = LAB_image.get_shared_channels(1,1);
    A_channel = (A_channel - LAB_color.A).sqr();
    B_channel = LAB_image.get_shared_channels(2,2);
    B_channel = (B_channel - LAB_color.B).sqr();
    difference_image = (L_channel + A_channel + B_channel).sqrt();

    //Return the negative of the image, so that higher number = more similar.
    return (255 - CImg<float>(difference_image.get_normalize(0,255)));
}
