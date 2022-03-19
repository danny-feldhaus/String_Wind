#include "path_calculation.h"
//g++  -Wall *.cpp -lm -lpng -lpthread -Dcimg_display=0 -o main.exe -DDEBUG -lstdc++fs
string_wind::path_calculator::path_calculator(string_wind::path_parameters* params)
{
    parameters = params;

    assert_parameters();

    full_image = CImg<float>(parameters -> file_name);
    //The with of the image, with each pixel the width of a string
    int image_width = static_cast<int>(std::round(params -> canvas_size_in_feet / (params -> string_width_in_mm/305)));
    float height_width_ratio = static_cast<float>(full_image.height()) / full_image.width();
    int image_height = image_width * height_width_ratio;

    //The ratio of the string's width (one pixel) to the image's width.
    unit_string_width = 1.0 / image_width;
    //re-size the image to the correc timage width.
    full_image.resize(image_width,image_height);

    //Convert the given unit points (0-1) to the scale of the given image (0-modified_image.width())
    calculate_local_points(parameters -> pins,full_image,local_pins);
    //Calculate the mask for the image. All pixels with value (0,0,0) are not included in the mask, and not used in the lengths / grades calculations.
    //TODO: CURRENTLY, MASKS ARE NOT IMPLEMENTED FOR FULL-COLOR IMAGES.
    mask  = CImg<float>(full_image.height(), full_image.width(), 1,1);
    image_methods::Calculate_Mask(full_image, mask);

    for(color_RGB<float> color : parameters -> colors)
    {
        color_difference_maps.push_back(image_methods::Get_Color_Similarity(full_image, color));
        path_instances.push_back(path_instance(&color_difference_maps.back(),parameters,&local_pins,&mask));
    }

    #if DEBUG
        std::cout << "Created " << path_instances.size() << " path instances.\n";
        for (auto& path: std::experimental::filesystem::directory_iterator("Output_Images"))
        {
            std::experimental::filesystem::remove_all(path);
        }
        for(int i=0; i < (int)color_difference_maps.size(); i++)
        {
            //Clear the output_images directory

            color_difference_maps[i].save(("Output_Images/color_difference_" + std::to_string(parameters -> colors[i].get_r()) + "," +
                                                                               std::to_string(parameters -> colors[i].get_g())  + "," +
                                                                               std::to_string(parameters -> colors[i].get_b()) + ".png").c_str());
        }
    #endif
}

string_wind::path_calculator::~path_calculator()
{
    local_pins.clear();
    color_difference_maps.clear();
    path_instances.clear();
}

void string_wind::path_calculator::calculate_local_points(const vector<point<float>>& unit_points,const CImg<float>& image, vector<point<int>>& output_points)
{
    for(point<float> up : unit_points)
    {
        output_points.push_back(image_methods::Unit_To_Image(up,image));
    }
}

void string_wind::path_calculator::calculate_path()
{   
    bool continue_calculating = true;
    pin_and_grade p_a_g;
    int pin_num = 0;
    int best_pin = -1;
    float best_grade = -1;
    int best_path_instance;
    #if DEBUG
    std::cout << "Calculating Path...\n";
    #endif
    while(continue_calculating)
    {
        best_grade = -1;
        best_pin = -1;
        std::cout << "Step #" << pin_num << '\n';
        for(int i=0; i< (int)path_instances.size(); i++)
        {
            #if DEBUG
            //std::cout << "Prev pin for color " << i << ": " << path_instances[i].get_cur_pin() << '\n';
            #endif
            p_a_g = path_instances[i].get_next_best_pin_and_score();
            std::cout << "Grade for " << i << ": " << p_a_g.grade << '\n'; 
            if(p_a_g.grade > best_grade)
            {
                best_grade = p_a_g.grade;
              //  std::cout << "New Best Grade: " << best_grade << '\n';
                best_pin = p_a_g.pin;
                best_path_instance = i;
            }
        }
        if(best_grade < (parameters -> darkness_threshold)) 
        {
            continue_calculating = false;
        }
        else
        {
            #if DEBUG
            std::cout << "Moving from pin " << path_instances[best_path_instance].get_cur_pin() << " to " << best_pin << " with grade " << best_grade << " and color #" << best_path_instance << '\n'; 
            #endif
            path.push_back(path_step(path_instances[best_path_instance].get_cur_pin(),best_pin,parameters -> colors[best_path_instance]));
            path_instances[best_path_instance].move_to_next();
            #if DEBUG
            if((pin_num + 1) % 1000 == 0)
            {
                std::cout << "Saving Progress at " + std::to_string(pin_num) + " steps.\n";
                CImg<float> progress_img = draw_strings();
                progress_img.save(("Output_Images/" + std::to_string(pin_num) + "_line_progress.png").c_str());
            }
            #endif
            pin_num++;
        }
    }
}

CImg<float> string_wind::path_calculator::draw_strings()
{
    CImg<float> out_image(full_image.width(), full_image.height(), 1, 3, 128.0);
    out_image.save("out_image_before_loop.png");
    vector<vector<int>> paths;
    vector<int>* cur_path;
    vector<int> path_lengths;
    vector<int> frequencies;
    point<int> point_a, point_b;
    int max_length = 0;
    int path_size;
    int path_index;

    for(path_instance p_i : path_instances)
    {
        paths.push_back(p_i.get_path());
        max_length = max(max_length, (int)paths.back().size());
    }
    for(int i=0; i<max_length-1; i++)
    {
        for(int j=0; j<(int)paths.size(); j++)
        {
            cur_path = &paths[j];
            path_size = static_cast<int>(cur_path -> size());
            if(path_size >= max_length - i)
            {
                path_index = (i - (max_length - path_size)) ;
                std::cout << "Index: " << path_index << '\n'; 
                #if DEBUG
                std::cout << "Path Size: " << path_size << ", Index: " << path_index << '\n'; 
                #endif
                point_a = local_pins[(*cur_path)[path_index]];
                point_b = local_pins[(*cur_path)[path_index+1]];
                out_image.draw_line(point_a.x,point_a.y,point_b.x,point_b.y,parameters -> colors[j].get_array());

            }
        }
    }

    out_image.save("outimage.png");
    return out_image;
}

void string_wind::path_calculator::load_from_parameters()
{
    //Assert that the file name exists
    assert(parameters -> file_name);
    //Assert that all pins are within the bounds of the image
    assert(parameters -> canvas_size_in_feet > 0);
    //Assert that the string width is a viable size. (greater than zero, less than the size of the image). Realistically, string size should never be that big, but the code will still run with that value. 
    assert(parameters -> string_width_in_mm > 0 && parameters -> string_width_in_mm / 305.0 < parameters -> canvas_size_in_feet);
    //Calculate the new image width, with the width of the pixel equal to the width of the string.
    assert(parameters -> pins.size() > 0);
    //Assert that at least one color was given
    assert(parameters -> colors.size() > 0);

    //Assert that all pins are within unit range
    for(point p : parameters -> pins)
    {
        assert(p.x >= 0);
        assert(p.x <= 1);
        assert(p.y >= 0);
        assert(p.y <= 1);
    }

    
}


string_wind::path_instance::path_instance(CImg<float>* _values, path_parameters* _parameters, vector<point<int>>* _local_points, CImg<float>* _mask)
{
    values = _values;
    parameters = _parameters;
    local_points = _local_points;
    mask = _mask;
    pin_count = static_cast<int>(local_points -> size());
    calculate_initial_grades();
    path.push_back(0);
}


int string_wind::path_instance::move_to_next()
{
    int prev_pin = path.back();
    point<int> prev_point = (*local_points)[prev_pin];
    int next_pin = -1;
    point<int> next_point;
    float a_b_grade, a_b_length;
    float cur_slope, a_b_slope, tangent, overlapped_pixels;
    float sum, lost_amount;
    point<int> point_a, point_b;


    pin_and_grade next_pin_and_grade = get_next_best_pin_and_score();
    next_pin = next_pin_and_grade.pin;
    cur_best_score = next_pin_and_grade.grade;
    next_point = (*local_points)[next_pin];
    #if DEBUG
    std::cout << "\tMoving from " << prev_pin << " to " << next_pin << '\n';
    #endif
    path.push_back(next_pin);

    //Update the scores based on the new line.
    for(int index_a = 0; index_a < pin_count; index_a++)
    {
        point_a = (*local_points)[index_a];
        for(int index_b = index_a+1; index_b < pin_count; index_b++)
        {
            point_b = (*local_points)[index_b];
            a_b_length = path_lengths(index_a, index_b);
            a_b_grade = path_grades(index_a, index_b);
            //If this line intersects with the new line, darken the overlapping pixels.
            if(a_b_length && line_methods::do_intersect(point_a, point_b, prev_point, next_point))
            {
                #if DEBUG
                //std::cout << "Calculating slope between " << prev_point << " and " << next_point << ".\n";
                #endif
                cur_slope = prev_point.slope_to(next_point);
                #if DEBUG
                //std::cout << "Slope: " << cur_slope << '\n';
                //std::cout << "Calculating slope between " << point_a << " and " << point_b << ".\n";
                #endif
                a_b_slope = point_a.slope_to(point_b);
                #if DEBUG
                // std::cout << "Slope: " << a_b_slope << '\n';
                #endif
                tangent = fabs((cur_slope - a_b_slope)/(1.0 + cur_slope*a_b_slope));
                //If the tangent is zero, the two lines are the same.
                if(tangent == 0)
                {
                    overlapped_pixels = a_b_length;
                }
                else
                {
                    overlapped_pixels = 1.0 / tangent;
                    //If the two lines overlap for longer than their length, set the overlap equal to the length. 
                    if(overlapped_pixels > a_b_length)
                    {
                        overlapped_pixels = a_b_length;
                    }
                }
                sum = a_b_grade * a_b_length;
                lost_amount = a_b_grade * (1.0 - parameters -> darkening_modifier) * overlapped_pixels;
                a_b_grade = (sum - lost_amount) / a_b_length;
                #if DEBUG
                //std::cout << "Grade Before: " << path_grades(index_a, index_b) << '\n';
                #endif
                path_grades(index_a, index_b) = a_b_grade;
                #if DEBUG
                //std::cout << "Grade After: " << path_grades(index_a, index_b) << '\n';
                #endif
            }
        }
    }
    return next_pin;
}

void string_wind::path_instance::calculate_initial_grades()
{
    path_grades = CImg<int>(pin_count, pin_count);
    path_lengths = CImg<int>(pin_count, pin_count);

    point<int> pin_a, pin_b;
    float mean;

    cur_best_score = 0;
    for(int index_a = 0; index_a < pin_count; index_a++)
    {
        pin_a = (*local_points)[index_a];
        for(int index_b = index_a+1; index_b < pin_count; index_b++)
        {
            pin_b = (*local_points)[index_b];
            mean = line_methods::get_line_average_value(pin_a, pin_b, values, mask);
            cur_best_score = max(cur_best_score, mean); 
            path_grades(index_a, index_b) = mean;
            path_lengths(index_a,index_b) = pin_a.distance_to(pin_b);
        }
    }
}

string_wind::pin_and_grade string_wind::path_instance::get_next_best_pin_and_score()
{
    //Find the next best pin.
    int prev_pin = path.back();
    int best_pin = -1;
    float best_grade = 0;
    float cur_grade;
    for(int i=0; i<pin_count; i++)
    {
        if(i != prev_pin)
        {
            cur_grade = path_grades(min(prev_pin,i),max(prev_pin,i));
            if(cur_grade > best_grade)
            {
                best_grade = cur_grade;
                best_pin = i;
            }
        }
    }
    #if DEBUG
    std::cout << "\tPrev Pin: " << prev_pin << ", Next Pin: " << best_pin << ", Grade: " << best_grade << '\n';
    #endif
    return pin_and_grade(best_pin,best_grade);
}

int string_wind::path_instance::get_cur_pin()
{
    return path.back();
}

vector<int>& string_wind::path_instance::get_path()
{
    return path;
}

