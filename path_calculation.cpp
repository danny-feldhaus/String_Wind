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

    //Load the image from the given location
    modified_image = CImg<unsigned char>(parameters -> file_name);
    assert(!input_image.empty());

    pin_count = static_cast<int>(parameters -> pins.size());
    assert(pin_count > 1);
    int image_width = static_cast<int>(std::round(params -> canvas_size_in_feet / (params -> string_width_in_mm/305)));
    unit_string_width = 1.0 / image_width;

    //Load the image from the given file.
    modified_image = CImg<unsigned char>(modified_image);
    //Convert the given unit points (0-1) to the scale of the given image (0-modified_image.width())
    calculate_local_points(parameters -> pins);
    //Calculate the mask for the image. All pixels with value (0,0,0) are not included in the mask, and not used in the lengths / grades calculations.
    image_methods::Calculate_Mask(modified_image,mask);
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
        (mask * 255).save("Output_Images/mask.png");
        modified_image.save("Output_Images/modified.png");
    #endif
    calculate_initial_grades();
}

string_wind::path_calculator::~path_calculator()
{
    for(int i=0; i< (int)parameters -> pins.size(); i++)
    {
        delete[] path_grades[i];
        delete[] path_lengths[i];
    }
    delete[] path_grades;
    delete[] path_lengths;
}

void string_wind::path_calculator::calculate_local_points(vector<point<float>>& unit_points)
{
    //std::cout << "Calculating local points for " << unit_points.size() << " pins.\n";
    int min_size = min(modified_image.width(), modified_image.height());
    for(point<float> up : unit_points)
    {
        local_pins.push_back(point<int>(min_size*up.x,min_size*up.y));
        //std::cout << up << '\n';
        //std::cout << local_pins.back() << '\n';
    }
}

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
            
                CImg<unsigned char> progress_image = draw_strings();
                progress_image.save(progress_file_name.c_str());
            }
        #endif
    } 
    while (best_next_pin != cur_pin && best_score > parameters -> darkness_threshold);
    return path;
}

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

CImg<unsigned char> string_wind::path_calculator::draw_strings()
{
    int image_width = static_cast<int>(std::round(parameters -> canvas_size_in_feet / (parameters -> string_width_in_mm/305.0)));
    int image_height = image_width / ((float)modified_image.width() / (float)modified_image.height());
    CImg<unsigned char> out_image(image_width,image_height, 1, parameters -> channels,255);
    #if DEBUG
    std::cout << "Path Length from draw_strings(): " << path.size() << '\n';
    #endif
    image_methods::Draw_Path(path,parameters -> pins, out_image);
    return out_image;
}