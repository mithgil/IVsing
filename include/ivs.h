
#ifndef IVS_H
#define IVS_H

#include <variant>    // Required for std::variant
#include <H5Cpp.h>    // For HDF5 operations
#include <iostream>   // For console I/O
#include <string>     // For std::string
#include <vector>     // For std::vector
#include <fstream>    // For file streams
#include <sstream>    // For std::stringstream
#include <algorithm>  // For std::sort, etc.
#include <filesystem> // C++17 filesystem operations
#include <optional>   // C++17 std::optional
#include <regex>      // For regular expressions
#include <tuple>      // For std::tuple
#include <cmath>      // For std::isnan, etc.
#include <map>        // For std::map
#include <json.hpp>   

using json = nlohmann::json;

struct Expdata{
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
    Expdata() = default;
};

struct ScanRangeData {
    std::tuple<double, double> scanRange;
    std::tuple<int, int> scanPixels;
    char unit[10];

    // This constructor ensures proper initialization when a ScanRangeData object is created
    ScanRangeData() : scanRange(0.0, 0.0), scanPixels(0, 0) {
        unit[0] = '\0'; // Initialize as empty string
    }
};

class datParser {
public: 
    //constructor
    
    datParser();

    void parse_metadata(const std::string& filePath);
    void correctDoubleBias();
    void parse_experiments(const std::string& input_directory);
    
    void output(const std::string& filename);
    
private:

    std::string experiments_type; 
    std::map<std::string, std::variant<int, std::string>> metadata;

    void writingJSON(const std::string& input_directory);
    void writingH5(const std::string& input_directory);

    void parse_tsv(std::ifstream& fp);
    std::vector<std::string> header;
    std::vector<std::vector<double>> ColumnData; //data not packed to each channel
    Expdata experiments_data;

    ScanRangeData scanRangeData;
    void writePixels(int& FileCounts);
    std::optional<std::pair<double, double>> extractXYCoordinates(const std::string& filepath);
    void extractFullScanRange(
        const std::string& input_directory,
        const std::vector<std::string>& sorted_dat_filenames
    );
    

};
    

#endif // IVS_H