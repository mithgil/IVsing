# if compiling with clang, use the following command:
# cmake -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_C_COMPILER=clang ..

cmake_minimum_required(VERSION 3.10)

project(ivs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 禁用編譯器擴展（保持標準化）

# Set the policy to NEW to use <PackageName>_ROOT variables
cmake_policy(SET CMP0074 NEW)

# Specify the path to HDF5, otherwise it will go to /opt/hdf5, the cross-compiled hdf5
set(HDF5_ROOT "/usr")

# Find HDF5 package (both C and C++ components)
find_package(HDF5 COMPONENTS C CXX REQUIRED)

include_directories(include)
# Include HDF5 header directories
include_directories(${HDF5_INCLUDE_DIRS})

# Add the library for your main source
#add_library(ivslib STATIC src/ivs.cpp)
add_library(ivslib SHARED 
    src/ivs.cpp
    src/utilities.cpp 
)

# Add the executable for your tests
add_executable(ivs_executable tests/main.cpp)

# Link libraries
target_link_libraries(ivs_executable ivslib ${HDF5_C_LIBRARIES} ${HDF5_CXX_LIBRARIES})
