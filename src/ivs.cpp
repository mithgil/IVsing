#include <H5Cpp.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem> // c++17 version
#include "ivs.h"

namespace fs = std::filesystem;

std::vector<std::vector<double>> getdata(const std::string& input_directory) {
    // Collect all .dat files in the input directory
    
    int file_count = 0;

    std::vector<std::string> dat_files;
    for (const auto& entry : fs::directory_iterator(input_directory)) {
        if (entry.path().extension() == ".dat") {
            dat_files.push_back(entry.path().filename().string());
            ++file_count; // Increment the file count
        }
    }

    // Sort the filenames
    std::sort(dat_files.begin(), dat_files.end());

    int pixels = 500;
    std::vector<std::vector<double>> iv_arr(pixels, std::vector<double>(file_count + 1, 0.0));

    for (std::size_t i = 0; i < dat_files.size(); ++i) {

        const auto& dat_file = dat_files[i];
        
        //open the file
        std::ifstream infile(input_directory + "/" + dat_file);
        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << dat_file << std::endl;
            continue;
        }

        std::string line;
        
        int line_num = 1; //number of line
        int line_header = 0; // number of line at header
        
        while (std::getline(infile, line)) {

            size_t found = line.find("[DATA]"); // the row after this line is the header

            if(found!=std::string::npos){
                line_header = line_num+1; // where bias \tcurrent
                // std::cout<<"header at line: "<<line_header<<std::endl;
            }

            if(line_header!=0 && line_num > line_header){ //skip the header line

                    size_t tabpos = line.find('\t'); //find position of "tab" (in a tsv file)
                    if(tabpos!=std::string::npos){
                        
                        std::string val1, val2;
                        // record first/second column val from position of 0~tabpos, tabpos~end
                        val1 = line.substr(0,tabpos); 
                        val2 = line.substr(tabpos+1,line.size()-tabpos-2); 
                        
                        if(i == 0){
                            // append voltage column for the first extraction

                            iv_arr[line_num-line_header-1][i] = std::stod(val1);
                            iv_arr[line_num-line_header-1][i+1] = std::stod(val2);
                        }else{
                            iv_arr[line_num-line_header-1][i+1] = std::stod(val2);
                        }
                    }
                }
            line_num += 1;
        }
        infile.close();
    }

    std::cout << "All data is loaded." << std::endl;

    return iv_arr;    
}

void writing(const std::string& filename, const std::string& dataset_name,
            const std::vector<std::vector<double>>& data, const std::vector<std::string>& headers) {
    // Open a new file using the default property lists.
    H5::H5File file(filename, H5F_ACC_TRUNC);

    // Create the data space for the dataset.
    hsize_t dims[2] = {static_cast<hsize_t>(data.size()), static_cast<hsize_t>(data[1].size())};
    H5::DataSpace dataspace(2, dims);

    // Flatten the 2D vector into a 1D array
    std::vector<double> flat_data;
    flat_data.reserve(data.size() * data[0].size());
    for (const auto& row : data) {
        flat_data.insert(flat_data.end(), row.begin(), row.end());
    }

    // Create the dataset.
    H5::DataSet dataset = file.createDataSet(dataset_name, H5::PredType::NATIVE_DOUBLE, dataspace);

    // Write the data to the dataset.
    dataset.write(flat_data.data(), H5::PredType::NATIVE_DOUBLE);

    // Create a group for headers
    H5::Group header_group = file.createGroup("/Headers");

    // Write headers as separate datasets
    for (size_t i = 0; i < headers.size(); ++i) {
        // Create dataspace and dataset for each header
        hsize_t header_dim[1] = {1};
        H5::DataSpace header_space(1, header_dim);
        H5::DataSet header_dataset = header_group.createDataSet(headers[i], H5::StrType(H5::PredType::C_S1), header_space);

        // Write header string to dataset
        header_dataset.write(headers[i].c_str(), H5::StrType(H5::PredType::C_S1));
    }
    
    std::cout << "Data written to " << filename << std::endl;

    // Close the HDF5 file
    file.close();
}


