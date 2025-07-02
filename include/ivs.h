
#ifndef IVS_H
#define IVS_H

#include <variant> // Required for std::variant
#include <H5Cpp.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem> // c++17 version
#include <tuple>
#include <map>
#include <json.hpp> // in include

using json = nlohmann::json;

struct expdata{
    std::vector<double> z;
    std::vector<double> bias;
    std::vector<std::vector<double>> current_fwd;
    std::vector<std::vector<double>> current_bwd;
    std::vector<std::vector<double>> tb_fwd;
    std::vector<std::vector<double>> tb_bwd;
    std::vector<std::vector<double>> amp_fwd;
    std::vector<std::vector<double>> amp_bwd;
    std::vector<std::vector<double>> phase_fwd;
    std::vector<std::vector<double>> phase_bwd;

};

struct ScanRangeData {
    std::tuple<double, double> scanRange;
    std::tuple<int, int> scanPixels;
    char unit[10];
};

class datParser {

public:
    using ScanRangeData = ::ScanRangeData;  

    datParser(){
        // Initialize expdata with empty vectors
        experiments_data.z = {};
        experiments_data.bias = {};
        experiments_data.current_fwd = {};
        experiments_data.current_bwd = {};
        experiments_data.amp_fwd = {};
        experiments_data.amp_bwd = {};
        experiments_data.phase_fwd = {};
        experiments_data.phase_bwd = {};
        experiments_data.tb_fwd = {};
        experiments_data.tb_bwd = {};    
    }

    void printColumnData();
    void printFirstDat(const std::string& filePath);
    void parse_metadata(const std::string& filePath);
    void correctDoubleBias();
    void parse_experiments(const std::string& input_directory);
    
    void output(const std::string& filename);
    void writingJSON(const std::string& input_directory, ScanRangeData& scanParams);
    void writingH5(const std::string& input_directory, ScanRangeData& scanParams);
    

private:

    std::string experiments_type; 
    std::map<std::string, std::variant<int, std::string>> metadata;

    void parse_tsv(std::ifstream& fp);
    std::vector<std::string> header;
    std::vector<std::vector<double>> ColumnData; //data not packed to each channel
    expdata experiments_data;

};
    

#endif // IVS_H