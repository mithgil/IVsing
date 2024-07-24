/*
A c++ project to combine a great number of iv curves dat file into a single h5

using c++17  HDF5

update: 2024/7/20
@Tim
*/

#include <iostream>
#include <vector>
#include <string>
#include <filesystem> //std::filesystem::path 
#include "ivs.h"

namespace fs = std::filesystem;

int main() {

    //the directory of the data 
    fs::path input_directory = "/home/tim/Documents/DATA/nanonis2/20240717/1u 100pix map2";
    // Append the file name to the directory path
    fs::path output_file = input_directory / "ivmap.h5";

    std::vector<std::vector<double>> ivdata;

    ivdata = getdata(input_directory.string());

    // Get the number of rows
    size_t num_rows = ivdata.size();

    // Get the number of columns (assuming all rows are of equal size)
    size_t num_cols = (num_rows > 0) ? ivdata[0].size() : 0;

    // Output the sizes
    std::cout << "Rows: " << num_rows <<std::endl
                << "Columns: " << num_cols << std::endl;

    // Generate headers
    std::vector<std::string> headers(num_cols);
    headers[0] = "Bias";
    for (int i = 1; i < num_cols; ++i) {
        headers[i] = "Current" + std::to_string(i);
    }

    // Write data with headers to HDF5
    writing(output_file.string(), "2DArray", ivdata, headers);

    return 0;
}
