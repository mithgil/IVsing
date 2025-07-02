// utilities.h
#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <cmath> // For std::sqrt
#include <string>         // Make sure to include this for std::string
#include <filesystem> // For std::filesystem


// Function to check if a number is a perfect square
bool isPerfectSquare(int number);

// Function to print help message
void printHelp();

// Function to process directories
void process_directories(const std::string& path_str);

#endif // UTILITIES_H