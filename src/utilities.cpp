
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
                    std::cout << " Processing subdirectory: " << entry.path().filename().string() << std::endl;

                    int file_count = 0;
                    for (const auto& sub_entry : fs::directory_iterator(entry.path())) {
                        if (sub_entry.path().extension() == ".dat") {
                            ++file_count;
                        }
                    }
                    std::cout << "  - " << "Number of dat files is " << file_count << std::endl;

                    if (isPerfectSquare(file_count) && file_count > 0) {
                        fs::path input_directory = entry.path().string();
                        datParser datparser;
                        datparser.parse_experiments(input_directory);
                        datparser.output(input_directory);
                    } else {
                        char option;

                        std::cout << "  - " << "(T.T) Oooops! the number of dat files is not a perfect square" << std::endl
                                  << "Are you sure you want to continue processing? [y/n]" << std::endl;

                        std::cin >> option; // Reads a single character

                        // Use a new scope for the switch block's variables
                        // to avoid "crosses initialization" errors
                        {
                            fs::path input_directory; // Declare here
                            datParser datparser;      // Declare here

                            switch (option) { 
                                case 'y':     // Works because 'y' is a char literal
                                case 'Y': 
                                    std::cout << "Continuing processing despite non-perfect square count." << std::endl;
                                    input_directory = entry.path().string();
                                    datparser.parse_experiments(input_directory);
                                    datparser.output(input_directory);
                                    break;
                                case 'n': 
                                case 'N': 
                                    std::cout << "Exiting without processing." << std::endl;
                                    break;
                                default:
                                    std::cerr << "Invalid option! Please enter [y/n]." << std::endl;
                                    break;
                            }
                        } // End of scope for input_directory and datparser
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