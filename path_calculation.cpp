#include "path_calculation.h"
//g++  -Wall *.cpp -lm -lpng -lpthread -Dcimg_display=0 -o main.exe -DDEBUG -lstdc++fs
string_wind::path_calculator::path_calculator(string_wind::path_parameters* params)
{
    parameters = params;
    //Assert that the file name exists
    assert(parameters -> file_name);
    //Assert that all pins are within the bounds of the image
    assert(parameters -> canvas_size_in_feet > 0);
    //Assert that the string width is a viable size. (greater than zero, less than the size of the image). Realistically, string size should never be that big, but the code will still run with that value. 
    assert(parameters -> string_width_in_mm > 0 && parameters -> string_width_in_mm / 305.0 < parameters -> canvas_size_in_feet);
    //Calculate the new image width, with the width of the pixel equal to the width of the string.
    assert(parameters -> pins.size() > 0);
    assert(parameters -> colors.size() > 0);

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
    image_methods::Calculate_Mask(full_image,mask);

    for(float* color : parameters -> colors)
    {
        color_difference_maps.push_back(image_methods::Get_Color_Similarity(full_image, color));
        path_instances.push_back(path_instance(&color_difference_maps.back(),parameters,&local_pins,&mask));
        grades.push_back(path_instances.back().get_best_score());
    }
    #if DEBUG
    std::cout << "Created " << path_instances.size() << " path instances.\n";
    for(int i=0; i < (int)color_difference_maps.size(); i++)
    {
        //Clear the output_images directory
        for (auto& path: std::experimental::filesystem::directory_iterator("Output_Images"))
        {
            std::experimental::filesystem::remove_all(path);
        }
        color_difference_maps[i].save(("Output_Images/color_difference_" + std::to_string(i) + ".png").c_str());
    }
    #endif

    /*
    #if DEBUG
        float* test_color = new float[3]{255,0,0};
        CImg<float> test_difference = image_methods::Get_Color_Similarity(modified_image,test_color).mul(mask);
        path_instance test_p_i = path_instance(&test_difference, parameters, &local_pins, &mask);
        delete[] test_color;
    #endif
    //Convert the modified image to grayscale, and invert it. This creates an image that represents the input image's darkness. Also, apply the mask.
    image_methods::Convert_To_Grayscale(modified_image, modified_image);
    modified_image = (255 - modified_image).mul(mask);

    //Assert that each unit pin given is within the 0-1 range. 
    for(int i=0; i< (int)params -> pins.size(); i++)
    {
        assert(params -> pins[i].x >= 0);
        assert(params -> pins[i].y >= 0);
        assert(params -> pins[i].x < 1);
        assert(params -> pins[i].y < 1);
    }    
    
    #if DEBUG
        for (auto& path: std::experimental::filesystem::directory_iterator("Output_Images"))
        {
            std::experimental::filesystem::remove_all(path);
        }
        test_difference.save("Output_Images/test_difference_2 .png"); 
        (mask * 255).save("Output_Images/mask.png");
        modified_image.save("Output_Images/modified.png");
    #endif
    calculate_initial_grades();
    */
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
        #if DEBUG
        std::cout << "Added local point " << output_points.back() << '\n';
        #endif
    }
}

void string_wind::path_calculator::calculate_path()
{   
    bool continue_calculating = true;
    vector<float> best_scores;
    int pin_num = 0;
    int next_pin;
    #if DEBUG
    std::cout << "Calculating Path...\n";
    #endif
    while(continue_calculating)
    {
        std::cout << "Step #" << pin_num << '\n';
        continue_calculating = false;
        for(int i=0; i< (int)path_instances.size(); i++)
        {
            std::cout << "#" << i <<": ";
            if(path_instances[i].get_best_score() > parameters -> darkness_threshold)
            {
                next_pin = path_instances[i].move_to_next();
                #if DEBUG
                std::cout << "\tPin #" << i << ": " << next_pin << '\n';
                std::cout << "\t\tGrade: " << path_instances[i].get_best_score() << '\n';
                #endif
                continue_calculating = true;
            }
        }
        if((pin_num + 1) % 1000 == 0)
        {
            std::cout << "Saving Progress at " + std::to_string(pin_num) + " steps.\n";
            CImg<float> progress_img = draw_strings();
            progress_img.save(("Output_Images/" + std::to_string(pin_num) + "_line_progress.png").c_str());
        }
        pin_num++;

    }
}

CImg<float> string_wind::path_calculator::draw_strings()
{
    CImg<float> out_image(full_image.width(), full_image.height(), 1, 3, 255.0);
    out_image.save("out_image_before_loop.png");
    vector<vector<int>> paths;
    vector<int>* cur_path;
    vector<int> path_lengths;
    vector<int> frequencies;
    point<int> point_a, point_b;
    int max_length;
    int path_index;
    for(int i=0; i < (int)path_instances.size(); i++)
    {
        paths.push_back(path_instances[i].get_path());
        path_lengths.push_back((int) path_instances[i].get_path().size());
    }
    max_length = *max_element(path_lengths.begin(), path_lengths.end());
    std::cout << "Max Length: " << max_length << '\n';
    for(int i=0; i< (int)paths.size(); i++)
    {
        frequencies.push_back((max_length) / path_lengths[i]);
    }
    for(int i=0; i< max_length-1; i++)
    {
        for(int j=0; j<(int)paths.size(); j++)
        {
            cur_path = &paths[j];

            if((i % frequencies[j]) == 0 && ((path_index + 1) < (int)(cur_path -> size())))
            {
                const float color[] = {parameters -> colors[j][0], parameters -> colors[j][1], parameters -> colors[j][2]};
                path_index = i / frequencies[j];
                point_a = local_pins[(*cur_path)[path_index]];
                point_b = local_pins[(*cur_path)[path_index+1]];
                out_image.draw_line(point_a.x,point_a.y,point_b.x,point_b.y,color);
            }
        }
    }
    out_image.save("outimage.png");
    return out_image;
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
    float best_grade = 0;
    float cur_grade;

    //Find the next best pin.
    for(int i=0; i<pin_count; i++)
    {
        if(i != prev_pin)
        {
            cur_grade = path_grades(min(prev_pin,i),max(prev_pin,i));
            if(cur_grade > best_grade)
            {
                best_grade = cur_grade;
                next_pin = i;
            }
        }
    }
    cur_best_score = best_grade;
    next_point = (*local_points)[next_pin];
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
                path_grades(index_a, index_b) = a_b_grade;
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

float string_wind::path_instance::get_best_score()
{
    return cur_best_score;
}

vector<int>& string_wind::path_instance::get_path()
{
    return path;
}



/*
vector<int> string_wind::path_calculator::calculate_path()
{
    path.clear();
    int cur_pin;
    //int common_size;
    vector<point<int>> common_points; 
    float best_score;
    int best_next_pin = 0;

    //The slopes of the two lines
    float m1, m2;
    //The tangent of the angle between the two lines
    float tangent; 
    //The number of pixels overlapping between the two lines (includes partial pixels)
    float overlap_amount;
    //The sum of the pixel values along the line (re-calculated from average)
    float sum;
    //The amount of darkness subtracted from the line
    float lost_amount;
    do
    {
        best_score = 0;
        cur_pin = best_next_pin;
        for(int i=0; i < pin_count; i++)
        {
            if(i != cur_pin && path_grades[min(cur_pin,i)][max(cur_pin,i)] > best_score)
            {
                best_score = path_grades[min(cur_pin,i)][max(cur_pin,i)];
                best_next_pin = i;
                #if DEBUG
                //std::cout << "New best pin: " << best_next_pin << " with score " << best_score << "\n";
                #endif
            }
        }
        path.push_back(best_next_pin);
        for(int i=0; i < pin_count; i++)
        {
            for(int j = i+1; j < pin_count; j++)
            {
                if(i != j && (int)path_lengths[i][j] != 0 && line_methods::do_intersect(local_pins[cur_pin], local_pins[best_next_pin], local_pins[i], local_pins[j]))
                {
                    #if DEBUG
                        //std::cout << i << ", " << j << ", " << cur_pin << ", " << best_next_pin << ": These lines intersect! Calculating...\n";
                    #endif
                    m1 = (float)(parameters -> pins[j].y - parameters -> pins[i].y) / (parameters -> pins[j].x - parameters -> pins[i].x);
                    m2 =  (float)(parameters -> pins[best_next_pin].y - parameters -> pins[cur_pin].y) / (parameters -> pins[best_next_pin].x - parameters -> pins[cur_pin].x);
                    tangent = fabs((m1 - m2)/(1.0+m1*m2));
                    overlap_amount = 1 / tangent;
                    //If the two lines are the same, or if they intersect for longer than the length of the evaluated line
                    if(tangent == 0 || overlap_amount > path_lengths[i][j])
                    {
                        overlap_amount = (float)path_lengths[i][j];
                    }
                    sum = path_grades[i][j] * path_lengths[i][j];
                    lost_amount = path_grades[i][j] * (1.0 - parameters -> darkening_modifier) * overlap_amount;
                    path_grades[i][j] = (sum - lost_amount) / path_lengths[i][j];
                    #if DEBUG 
                        //std::cout << "New grade: " << path_grades[i][j] << ", lost amount: " << lost_amount << '\n';
                        //std::cout << "There are " << overlap_amount << " common pixels between " << parameters -> pins[i] << " -> " << parameters -> pins[j] << " and " << parameters -> pins[cur_pin] << " -> " << parameters -> pins[best_next_pin] << '\n';
                        //if(overlap_amount >= 1)
                        //{
                        //     //std::cout << "There are " << overlap_amount << " overlap pixels between " << parameters -> pins[i] << " -> " << parameters -> pins[j] << " and " << parameters -> pins[cur_pin] << " -> " << parameters -> pins[best_next_pin] << '\n';
                        //     std::cout << "\tTangent: " << tangent << '\n';
                        //     std::cout << "\tm1: " << m1 << ", m2: " << m2 << '\n';
                        //}
                    #endif           

                }
            }
        }
        #if DEBUG
            std::cout << "Path Pin # " << path.size() << ": " << parameters -> pins[best_next_pin] << "(#" << best_next_pin << ")" << ", Score: " << best_score << "\n" ; 
            std::string progress_file_name = "Output_Images/" + std::to_string((int)path.size())+ "_pins.png";
            if((int)path.size() % 500 == 0)
            {
                std::cout << "Saving image to " << progress_file_name << ".\n";
            
                CImg<float> progress_image = draw_strings();
                progress_image.save(progress_file_name.c_str());
            }
        #endif
    } 
    while (best_next_pin != cur_pin && best_score > parameters -> darkness_threshold);
    return path;
}
*/
/*
void string_wind::path_calculator::calculate_initial_grades()
{
    point<int> pinA, pinB;
    float mean;

    path_grades = new float*[pin_count];
    path_lengths = new int*[pin_count];

    for(int i=0; i< pin_count; i++)
    {
        #if DEBUG
        std::cout << "Calculating grades for " << i << '\n';
        #endif
        //Create the arrays for grades and lengths 
        path_grades[i] = new float[pin_count];
        path_lengths[i] = new int[pin_count];
        pinA = local_pins[i];
        for(int j=i+1; j < pin_count; j++)
        {
            if(i != j)
            {
                pinB = local_pins[j];
                mean = line_methods::get_line_average_value(pinA,pinB,&modified_image,&mask);
                path_grades[i][j] = mean;
                //Estimate the length of the line with the pythagorean theorem
                path_lengths[i][j] =  sqrt(pow(local_pins[j].x-local_pins[i].x,2) + pow(local_pins[j].y - local_pins[i].y,2));
                
                #if DEBUG
                    //std::cout << "  Mean at " << pinA << " -> " << pinB << ": " << mean << '\n';
                    //std::cout << "  Length at (" << pinA << " -> " << pinB << ": " << path_lengths[i][j] << '\n';
                #endif
            }
        }
    }
}
*/
