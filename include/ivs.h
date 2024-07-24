

#ifndef IVS_H
#define IVS_H

#include <H5Cpp.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem> // c++17 version

namespace fs = std::filesystem;

std::vector<std::vector<double>> getdata(const std::string& input_directory);

void writing(const std::string& filename, const std::string& dataset_name,
            const std::vector<std::vector<double>>& data, const std::vector<std::string>& headers);
    

#endif // IVS_H

