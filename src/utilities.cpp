
// utilities.cpp
#include "utilities.h" 
#include "ivs.h"

namespace fs = std::filesystem;

// Function to check if a number is a perfect square
bool isPerfectSquare(int number) {
    if (number < 0) return false;
    int root = static_cast<int>(std::sqrt(number));
    return (root * root == number);
}

void printHelp() {
    std::cout << "Usage: ./ivs_executable [options] <file_directory>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help       Show this help message\n";
    std::cout << "  -v, --verbose    Enable verbose output\n\n";
    std::cout << "Example: ./ivs_executable /path/to/main/directory\n"; 
    std::cout << "note: /path/to/main/directory is the parent directory to subdirectoy of I-V maps\n"; 
}

void process_directories(const std::string& path_str){

    fs::path path(path_str);

    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            std::cout << "Subdirectories in " << path << ":\n";
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_directory(entry.path())) {
                    std::cout << "--------------------------------------------------" << std::endl;
                    std::cout << "  Processing subdirectory: " << entry.path().filename().string() << std::endl;

                    int file_count = 0;
                    for (const auto& sub_entry : fs::directory_iterator(entry.path())) {
                        if (sub_entry.path().extension() == ".dat") {
                            ++file_count;
                        }
                    }

                    if (isPerfectSquare(file_count) && file_count > 0) {
                        fs::path input_directory = entry.path().string();
                        datParser datparser;
                        
                        datparser.parse_experiments(input_directory);
                        datparser.output(input_directory);
                    } else {
                        std::cout << "  - " << "Warning: It looks like your map is not a square. Process breaks" << std::endl;
                    }
                }
            }
        } else {
            std::cerr << "Path does not exist or is not a directory.\n";
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}