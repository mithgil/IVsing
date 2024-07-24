cmake_minimum_required(VERSION 3.10)

project(ivs)

# Set the C++ standard to C++17 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find HDF5 package (both C and C++ component
find_package(HDF5 COMPONENTS C CXX REQUIRED)

include_directories(include)
# Include HDF5 header directories
include_directories(${HDF5_INCLUDE_DIRS})

# Add the executable for your tests
add_executable(test headers.cpp)

target_link_libraries(test ${HDF5_C_LIBRARIES} ${HDF5_CXX_LIBRARIES})
