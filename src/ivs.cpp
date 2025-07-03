#include <iostream>
#include <algorithm> // std::find_if
#include <cstring> // For strncpy
#include <limits>
#include "ivs.h"

namespace fs = std::filesystem;

datParser::datParser() {}

void datParser::parse_experiments(const std::string& input_directory){
    // Collect all .dat files in the input directory
    
    int file_count = 0;

    std::vector<std::string> dat_files;
    for (const auto& entry : fs::directory_iterator(input_directory)) {
        if (entry.path().extension() == ".dat") {
            dat_files.push_back(entry.path().filename().string());
            ++file_count; // Increment the file count
        }
    }

    // sort the filenames
    std::sort(dat_files.begin(), dat_files.end());

    writePixels(file_count);

    //open the file and parse metadata first
    parse_metadata(input_directory + "/" + dat_files[0]); //line of header, header (vector), dataSize (pixels on bias sweep)

    extractFullScanRange(input_directory, dat_files);

    // correct repeated column (which causes false column size) 
    correctDoubleBias();

    // check data content
    size_t biasfound = dat_files[0].find("Bias");
    size_t zspecfound = dat_files[0].find("Z-Spectroscopy");

    if (biasfound != std::string::npos){
        std::cout<<"\t- "<<"Bias-Spectroscopy: ";
        size_t OCfound = header[1].find("OC");// e.g. the header OC D1 Phase (deg) 

        if(OCfound != std::string::npos){ // "OC" found: Bias-displacement-current
            if(header.size() == 3){
                std::cout<<"ACCurrent-"
                            <<"forward only"<<std::endl;
                experiments_type = "Bias-Spectroscopy-ACCurrent-Map-fwd";
            } else if(header.size() == 5){
                std::cout<<"ACCurrent-"
                        <<"forward and backward"<<std::endl;
                experiments_type = "Bias-Spectroscopy-ACCurrent-Map-fbwd";
            }
        } else { // "OC" not found : DC current
            
            if(header.size() == 2){
                std::cout<<"Current-"
                        <<"forward only"<<std::endl;
                experiments_type = "Bias-Spectroscopy-Current-Map-fwd";
            }else if(header.size() == 3){
                std::cout<<"Current-"
                        <<"forward and backward"<<std::endl;
                experiments_type = "Bias-Spectroscopy-Current-Map-fbwd";
            }
        }
        
    } else if(zspecfound != std::string::npos){
        std::cout<<"  - "<<"Z-Spectroscopy Map";
        
        auto curit = std::find_if(header.begin(), header.end(), [](const std::string& str) {
            return str.find("Current") != std::string::npos;
        });

        auto inpit = std::find_if(header.begin(), header.end(), [](const std::string& str) {
            return str.find("Input 4") != std::string::npos;
        });

        if(curit != header.end() && inpit != header.end() && header.size() == 5 ){
            std::cout<<": forward and backward"<<std::endl;
            experiments_type = "Z-Spectroscopy-Map-fbwd";
        } else if (curit != header.end() && header.size() == 3){
            std::cout<<": forward only"<<std::endl;
            experiments_type = "Z-Spectroscopy-Map-fwd";
        } else{
            std::cout<<"more channels to be paresd"<<std::endl;
        }

    }else{
        std::cout<<"Unknown experiments";
        return; 
    }

    bool is_expdata_initialized = false;  

    int num_rows = std::get<int>(metadata["dataSize"]);

    // Initialize ivdata based on the size of the first file's data
    if (!is_expdata_initialized) {

        if( experiments_type == "Z-Spectroscopy-Map-fbwd"){
            experiments_data.z.resize(num_rows);
            experiments_data.current_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.current_bwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.tb_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.tb_bwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 

        } else if (experiments_type == "Z-Spectroscopy-Map-fwd"){
            experiments_data.z.resize(num_rows);
            experiments_data.current_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.tb_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
        } else if (experiments_type == "Bias-Spectroscopy-Current-Map-fwd"){
            experiments_data.bias.resize(num_rows);
            experiments_data.current_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
        } else if (experiments_type == "Bias-Spectroscopy-Current-Map-fbwd"){
            experiments_data.bias.resize(num_rows);
            experiments_data.current_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.current_bwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
        } else if (experiments_type == "Bias-Spectroscopy-ACCurrent-Map-fbwd"){
            experiments_data.bias.resize(num_rows);
            experiments_data.amp_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.amp_bwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.phase_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.phase_bwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 

        } else if (experiments_type == "Bias-Spectroscopy-ACCurrent-Map-fwd"){
            experiments_data.bias.resize(num_rows);
            experiments_data.amp_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
            experiments_data.phase_fwd.resize(num_rows, std::vector<double>(dat_files.size(), 0.0)); 
        }
        is_expdata_initialized = true;
    }
        
    if(is_expdata_initialized){

        //iterate over different names in a dir to parse data
        for (std::size_t i = 0; i < dat_files.size(); ++i) {

            const auto& dat_file = dat_files[i];
            
            //open the file
            std::ifstream infile(input_directory + "/" + dat_file);
            if (!infile.is_open()) {
                std::cerr << "Error opening file: " << dat_file << std::endl;
                break;
            }

            parse_tsv(infile); // to get columns values to ColumnData

            //data packing
            if( experiments_type == "Z-Spectroscopy-Map-fbwd"){
                
                for(size_t j = 0; j < num_rows;++j){
                    
                    experiments_data.z[j] = ColumnData[0][j];
                    experiments_data.current_fwd[j][i] = ColumnData[1][j];
                    experiments_data.tb_fwd[j][i] = ColumnData[2][j];
                    experiments_data.current_bwd[j][i] = ColumnData[3][j];
                    experiments_data.tb_bwd[j][i] = ColumnData[4][j];

                }

            } else if (experiments_type == "Z-Spectroscopy-Map-fwd"){
                for(size_t j = 0; j < num_rows;++j){

                    experiments_data.z[j] = ColumnData[0][j];
                    experiments_data.current_fwd[j][i] = ColumnData[1][j];
                    experiments_data.tb_fwd[j][i] = ColumnData[2][j];
                    
                }
            } else if (experiments_type == "Bias-Spectroscopy-Current-Map-fbwd"){
                for(size_t j = 0; j < num_rows;++j){

                    experiments_data.bias[j] = ColumnData[0][j];
                    experiments_data.current_fwd[j][i] = ColumnData[1][j];
                    experiments_data.current_bwd[j][i] = ColumnData[2][j];
                    
                }
            } else if (experiments_type == "Bias-Spectroscopy-Current-Map-fwd"){
                for(size_t j = 0; j < num_rows;++j){
                    experiments_data.bias[j] = ColumnData[0][j];
                    experiments_data.current_fwd[j][i] = ColumnData[1][j];
                }
            } else if (experiments_type == "Bias-Spectroscopy-ACCurrent-Map-fbwd"){
                for(size_t j = 0; j < num_rows;++j){
                    experiments_data.bias[j] = ColumnData[0][j];
                    
                    experiments_data.phase_fwd[j][i] = ColumnData[1][j];
                    experiments_data.amp_fwd[j][i] = ColumnData[2][j];
                    experiments_data.phase_bwd[j][i] = ColumnData[3][j];
                    experiments_data.amp_bwd[j][i] = ColumnData[4][j];
                
                }
            } else if (experiments_type == "Bias-Spectroscopy-ACCurrent-Map-fwd"){
                for(size_t j = 0; j < num_rows;++j){
                    experiments_data.bias[j] = ColumnData[0][j];
                    
                    experiments_data.phase_fwd[j][i] = ColumnData[1][j];
                    experiments_data.amp_fwd[j][i] = ColumnData[2][j];
                }
            }
            infile.close();
        }
    }
}

// utility function to trim leading and trailing whitespace from a string

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t");
    auto end = str.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

void datParser::correctDoubleBias() {
    if (header.empty()) {
        std::cerr << "Error: Header is empty. Cannot process 'Bias' columns." << std::endl;
        return;
    }

    int biasCount = 0;
    std::vector<int> biasIndices;

    for (int i = 0; i < header.size(); ++i) {
        if (header[i].find("Bias") != std::string::npos) {
            biasCount++;
            biasIndices.push_back(i);
        }
    }

    if (biasCount == 0) {
        std::cerr << "Warning: No 'Bias' column found in the header." << std::endl;
        return;

    } else if (biasCount > 1){
        std::cout << "  Alert: 'Bias' appears " << biasCount << " times at column indices: ";
        for (const auto& idx : biasIndices) {
            std::cout << idx << " ";
        }
        std::cout << std::endl;

        bool zeroExists = (std::find(biasIndices.begin(), biasIndices.end(), 0) != biasIndices.end());
        bool oneExists = (std::find(biasIndices.begin(), biasIndices.end(), 1) != biasIndices.end());

        if (zeroExists && !oneExists) {
            if (header.size() >= 2) {
                header.resize(2);
                std::cout << "    Alert: Header column corrected, resized to 2." << std::endl;
            } else {
                std::cerr << "    Error: Cannot resize header, insufficient size." << std::endl;
            }
        }
    }
}

/* parse and store metadata
    returns 
    1. line of header 
    2. datasize
    3. header as column vector
*/

void datParser::parse_metadata(const std::string& filePath) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        return;
    }

    std::string line;
    bool dataSectionFound = false;
    int line_num = 1; // Number of lines read
    int line_header = 0; // Line number where the header starts

    std::vector<std::string> columnvector;

    // Now continue reading the file
    while (std::getline(file, line)) {
        //line = trim(line);  // Clean up whitespace

        // Check for "Saved Date"
        if (line.find("Saved Date") != std::string::npos) {
            // Extract the value after "Saved Date"
            size_t pos = line.find('\t'); // Look for the tab delimiter
            std::string savedDate;

            if (pos != std::string::npos) {
                savedDate = line.substr(pos + 1); // Get the substring after the tab
                
                // Trim the time part if it exists
                size_t spacePos = savedDate.find(' '); // Look for the space before the time
                if (spacePos != std::string::npos) {
                    savedDate = savedDate.substr(0, spacePos); // Keep only the date
                }
            }
            
            metadata["ExpDate"] = savedDate;
        }
        
        if (line.find("[DATA]") != std::string::npos) {
            // Found the [DATA] section
            dataSectionFound = true;

            // Capture the next line as the header
            if (std::getline(file, line)) {
                line_header = line_num + 1;  // Set header line

                std::istringstream headerStream(line);
                std::string column;

                header.clear();
                while (std::getline(headerStream, column, '\t')) {
                    header.push_back(column);
                }
                
            } else {
                std::cerr << "Error: No header line found after [DATA] in file " << filePath << std::endl;
                break;
            }
        }
        line_num++;
    }

    if (!dataSectionFound) {
        std::cerr << "Error: [DATA] section not found in file " << filePath << std::endl;
    }

    metadata["lineofHeader"] = line_header;

    metadata["dataSize"] = static_cast<int>(line_num - line_header);// Points on bias sweep
        
    file.close();
}

/**
   utility to read tsv data dynamically
   returns columns of data
*/

void datParser::parse_tsv(std::ifstream& fp) {
    std::string line;
    std::vector<std::vector<double>> columns; // Will hold all columns

    int line_num = 1; //number of line
    int line_header = std::get<int>(metadata["lineofHeader"]); // number of line at header

    if(line_header == 0)
        std::cerr<< "Please parse metadata first!"<<std::endl;

    // Skip the header lines
    for (int ii = 0; ii < line_header; ++ii)
        std::getline(fp, line);

    while (std::getline(fp, line)) {
        std::istringstream iss(line);
        std::string value;
        size_t colIndex = 0;

        // Read values dynamically
        while (std::getline(iss, value, '\t')) {
            try {
                // Resize the columns vector if necessary
                if (colIndex >= columns.size()) {
                    columns.resize(colIndex + 1); // Add a new column
                }
                columns[colIndex].push_back(std::stod(value));
                colIndex++;

            } catch (const std::invalid_argument&) {
                std::cerr << "Error parsing value: " << value << " on line " << line_num << std::endl;
            }
        }
    }
    ColumnData = columns; //private member
}

void datParser::writePixels(int& FileCounts){

    double sqrt_file_count = std::sqrt(static_cast<double>(FileCounts));
    
    int xpixels = static_cast<int>(std::round(sqrt_file_count));
    int ypixels = static_cast<int>(std::round(sqrt_file_count));

    scanRangeData.scanPixels = std::make_tuple(xpixels, ypixels); 

}


std::optional<std::pair<double, double>> datParser::extractXYCoordinates(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        // std::cerr << "Error: Could not open file '" << filepath << "'." << std::endl;
        return std::nullopt;
    }

    std::regex x_regex(R"(X \(m\)\s*([-+]?\d+\.?\d*(?:[EeMmPp][-+]?\d*)?))");
    std::regex y_regex(R"(Y \(m\)\s*([-+]?\d+\.?\d*(?:[EeMmPp][-+]?\d*)?))");

    std::string line;
    std::smatch match;

    std::optional<double> x_coord;
    std::optional<double> y_coord;

    int line_count = 0;
    const int MAX_HEADER_LINES = 20;

    while (std::getline(file, line) && line_count < MAX_HEADER_LINES) {
        if (!x_coord.has_value() && std::regex_search(line, match, x_regex)) {
            if (match.size() > 1) {
                try { x_coord = std::stod(match[1].str()); }
                catch (const std::exception& e) { std::cerr << "Warning: Bad X value in " << filepath << std::endl; }
            }
        }
        if (!y_coord.has_value() && std::regex_search(line, match, y_regex)) {
            if (match.size() > 1) {
                try { y_coord = std::stod(match[1].str()); }
                catch (const std::exception& e) { std::cerr << "Warning: Bad Y value in " << filepath << std::endl; }
            }
        }
        if (x_coord.has_value() && y_coord.has_value()) {
            break;
        }
        line_count++;
    }
    file.close();

    if (x_coord.has_value() && y_coord.has_value()) {
        return std::make_pair(x_coord.value(), y_coord.value());
    } else {
        // std::cerr << "Warning: Could not find both X and Y in '" << filepath << "'." << std::endl;
        return std::nullopt;
    }
}

void datParser::extractFullScanRange(
    const std::string& input_directory,
    const std::vector<std::string>& sorted_dat_filenames)
{
    if (sorted_dat_filenames.empty()) {
        std::cout << "No .dat files found to extract boundary coordinates." << std::endl;
        return;
    }

    double firstX = 0.0;
    double secondX = 0.0;
    double pixelSize = 0.0;
    double firstY = 0.0;
    double lastX = 0.0;
    double lastY = 0.0;

    double xRange = 0.0;
    double yRange = 0.0;

    std::cout<<"\t- Scan pixels: ("<<std::get<0>(scanRangeData.scanPixels)<< ", "
                                <<std::get<1>(scanRangeData.scanPixels)<< ")"<< std::endl;

    // Extract from the first file
    std::string first_filepath = fs::path(input_directory) / sorted_dat_filenames.front();
    std::string second_filepath = fs::path(input_directory) / sorted_dat_filenames[1];
    // std::string secondrow_filepath = fs::path(input_directory) / sorted_dat_filenames[std::get<0>(scanRangeData.scanPixels)];

    auto first_coords_opt = extractXYCoordinates(first_filepath);
    if (first_coords_opt.has_value()) {

        firstX = first_coords_opt->first; // Assigning the first value of the pair
        firstY = first_coords_opt->second; // Assigning the second value of the pair

    } else {
        std::cerr << "Warning: Could not extract X/Y from first file: " << sorted_dat_filenames.front() << std::endl;
    }

    auto second_coords_opt = extractXYCoordinates(second_filepath);
    if (second_coords_opt.has_value()) {

        secondX = second_coords_opt->first; // Assigning the first value of the pair

    } else {
        std::cerr << "Warning: Could not extract X/Y from first file: " << sorted_dat_filenames[1] << std::endl;
    }

    pixelSize = secondX - firstX; // assuming square pixel

    // Extract from the last file 
    if (sorted_dat_filenames.size() > 1) {
        std::string last_filepath = fs::path(input_directory) / sorted_dat_filenames.back();
        auto last_coords_opt = extractXYCoordinates(last_filepath);
        if (last_coords_opt.has_value()) {
            lastX = last_coords_opt->first;
            lastY = last_coords_opt->second;

        } else {
            std::cerr << "Warning: Could not extract X/Y from last file: " << sorted_dat_filenames.back() << std::endl;
        }
    } else {
        lastX = firstX;
        lastY = firstY;
    }

    xRange = lastX - firstX + pixelSize;
    yRange = lastY - firstY + pixelSize;

    std::cout<< "\t- Full Scan range (m): (" << xRange <<", "<< yRange << ")"<<std::endl;
    std::cout<<"\t- Pixel size (m):"<<pixelSize<<std::endl;

    scanRangeData.scanRange = std::make_tuple(xRange, yRange);  

    strncpy(scanRangeData.unit, "m", sizeof(scanRangeData.unit) - 1);
    scanRangeData.unit[sizeof(scanRangeData.unit) - 1] = '\0'; // Ensure null-termination

}

void datParser::output(const std::string& input_directory) {

    std::cout << "  -------  " << std::endl;
    writingJSON(input_directory);
    std::cout << "  -------  " << std::endl;
    writingH5(input_directory);

}

void datParser::writingJSON(const std::string& input_directory) {
    // 檢查輸入目錄是否存在
    if (!fs::exists(input_directory)) {
        std::cerr << "Error: Input directory does not exist: " << input_directory << std::endl;
        return;
    }

    // 創建 JSON 物件
    json j;

    // 添加 ExpDate（確保 metadata 中有 ExpDate）
    if (metadata.count("ExpDate")) {
        const auto& expDateVariant = metadata.at("ExpDate");
        if (std::holds_alternative<std::string>(expDateVariant)) {
            std::string expDate = std::get<std::string>(expDateVariant);
            j["ExpDate"] = expDate; 
        } else {
            std::cerr << "Warning: ExpDate found but is not a string in metadata for HDF5!" << std::endl;
        }
    } else {
        std::cerr << "Warning: ExpDate not found or invalid in metadata!" << std::endl;
    }

    // 處理 scanrange 和 scanpixels（拆解 tuple）
    auto [xrange, yrange] = scanRangeData.scanRange;
    auto [xpixels, ypixels] = scanRangeData.scanPixels;

    j["scanrange"]["x"] = xrange;
    j["scanrange"]["y"] = yrange;
    j["scanrange"]["unit"] = scanRangeData.unit;

    j["scanpixels"]["x"] = xpixels;
    j["scanpixels"]["y"] = ypixels;

    j["datasets"]["bias"] = experiments_data.bias;
    j["datasets"]["current_fwd"] = experiments_data.current_fwd;

    if (!experiments_data.z.empty()) {
        j["datasets"]["z"] = experiments_data.z;
    }

    // 寫入檔案
    fs::path output_file = fs::path(input_directory) / "ivmapsing.json";

    std::ofstream out(output_file);
    if (!out.is_open()) {
        std::cerr << "Failed to open file: " << output_file << std::endl;
        return;
    }

    out << j.dump(4); 
    if (out.fail()) {
        std::cerr << "Failed to write JSON to file: " << output_file << std::endl;
        out.close();
        return;
    }

    out.close();
    std::cout << "   JSON file successfully written to: " << output_file << std::endl;
}

void datParser::writingH5(const std::string& input_directory) {
    fs::path dir_path(input_directory);
    fs::path filename = dir_path / "ivmapsing.h5";

    // Open HDF5 file
    H5::H5File file(filename.string(), H5F_ACC_TRUNC);

    // ================== 元數據部分 ==================
    // 1. 寫入 ExpDate 屬性 (根目錄)
    if (metadata.count("ExpDate")) { // Check if key exists
        const auto& expDateVariant = metadata.at("ExpDate"); 
        if (std::holds_alternative<std::string>(expDateVariant)) { // Check if it holds a string
            std::string expDate = std::get<std::string>(expDateVariant); // Get the string value
            H5::StrType str_type(H5::PredType::C_S1, expDate.size() + 1); // +1 for null terminator
            H5::Attribute expDate_attr = file.createAttribute("ExpDate", str_type, H5::DataSpace(H5S_SCALAR));
            expDate_attr.write(str_type, expDate);
        } else {
            std::cerr << "Warning: ExpDate found but is not a string in metadata for HDF5!" << std::endl;
        }
    } else {
        std::cerr << "Warning: ExpDate not found in metadata for HDF5!" << std::endl;
    }

    // 2. 寫入 scanrange 組
    auto [xrange, yrange] = scanRangeData.scanRange;
    {
        H5::Group scanrange_group = file.createGroup("/scanrange");
        
        // 寫入 values 數據集
        double range_values[2] = {xrange, yrange};
        hsize_t dims[1] = {2};
        H5::DataSpace dataspace(1, dims);
        H5::DataSet ds_range = scanrange_group.createDataSet("values", H5::PredType::NATIVE_DOUBLE, dataspace);
        ds_range.write(range_values, H5::PredType::NATIVE_DOUBLE);

        // 寫入 unit 屬性
        H5::StrType str_type(H5::PredType::C_S1, 10);
        H5::Attribute unit_attr = scanrange_group.createAttribute("unit", str_type, H5::DataSpace(H5S_SCALAR));
        unit_attr.write(str_type, scanRangeData.unit);
    }

    // 3. 寫入 scanpixels 組
    auto [xpixels, ypixels] = scanRangeData.scanPixels;
    {
        H5::Group scanpixels_group = file.createGroup("/scanpixels");
        
        int pixel_values[2] = {xpixels, ypixels};
        hsize_t dims[1] = {2};
        H5::DataSpace dataspace(1, dims);
        H5::DataSet ds_pixels = scanpixels_group.createDataSet("values", H5::PredType::NATIVE_INT, dataspace);
        ds_pixels.write(pixel_values, H5::PredType::NATIVE_INT);
    }

    // ================== 實驗數據部分 ==================
    // 創建 data Group
    H5::Group data_group = file.createGroup("/data");

    // Lambda 函數：寫入數據到 /data Group 下
    auto write1DData = [&](const std::string& dataset_name, const std::vector<double>& data) {
        if (!data.empty()) {
            hsize_t dims[1] = {data.size()};
            H5::DataSpace dataspace(1, dims);
            H5::DataSet dataset = data_group.createDataSet(dataset_name, H5::PredType::NATIVE_DOUBLE, dataspace);
            dataset.write(data.data(), H5::PredType::NATIVE_DOUBLE);
        }
    };

    auto write2DData = [&](const std::string& dataset_name, const std::vector<std::vector<double>>& data) {
        if (!data.empty() && !data[0].empty()) {
            hsize_t dims[2] = {data.size(), data[0].size()};
            H5::DataSpace dataspace(2, dims);
            std::vector<double> flat_data;
            for (const auto& row : data) {
                flat_data.insert(flat_data.end(), row.begin(), row.end());
            }
            H5::DataSet dataset = data_group.createDataSet(dataset_name, H5::PredType::NATIVE_DOUBLE, dataspace);
            dataset.write(flat_data.data(), H5::PredType::NATIVE_DOUBLE);
        }
    };

    // 寫入所有實驗數據集
    write1DData("z", experiments_data.z);
    write1DData("bias", experiments_data.bias);
    write2DData("current_fwd", experiments_data.current_fwd);
    write2DData("current_bwd", experiments_data.current_bwd);
    write2DData("tb_fwd", experiments_data.tb_fwd);
    write2DData("tb_bwd", experiments_data.tb_bwd);
    write2DData("amp_fwd", experiments_data.amp_fwd);
    write2DData("amp_bwd", experiments_data.amp_bwd);
    write2DData("phase_fwd", experiments_data.phase_fwd);
    write2DData("phase_bwd", experiments_data.phase_bwd);

    file.close();
    std::cout << "   HDF5 file successfully written to: " << filename << std::endl;

    std::cout << "    HDF5 file structured as:\n"
                << "      /ExpDate\n"
                << "      /scanrange/\n"
                << "      ├──unit\n"
                << "      ├──x\n"
                << "      └──y\n"
                << "      /scanpixels/\n"
                << "      ├──x\n"
                << "      └──y\n"
                << "      /datasets/\n"
                << "      ├──z (optional)\n"
                << "      ├──bias\n"
                << "      └──current_fwd\n" <<std::endl;

}

