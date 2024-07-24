cmake_minimum_required(VERSION 3.10)

project(ivs)

# Set the C++ standard to C++17 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Detect if we are cross-compiling for Windows
if(CMAKE_CROSSCOMPILING)

		# Set the toolchain file
		set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/x86_64-w64-mingw32.toolchain.cmake)

		# Add the cross-compiled HDF5 library path
		set(HDF5_ROOT /opt/hdf5-1.14)
		find_package(HDF5 REQUIRED COMPONENTS CXX)
		include_directories(${HDF5_INCLUDE_DIRS})

		# Print the HDF5 variables to verify
		message(STATUS "HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}")
		message(STATUS "HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
		message(STATUS "HDF5_CXX_LIBRARIES: ${HDF5_CXX_LIBRARIES}")

		include_directories(include) 

		# Add the library for your main source
		add_library(ivslib STATIC src/ivs.cpp) # STATIC does not become shared library

		# Specify the directory containing the HDF5 libraries
		link_directories(/opt/hdf5-1.14/lib)

		# Add the executable for your tests
		add_executable(ivs_executable tests/main.cpp)

		# Include directories
		target_include_directories(ivslib PRIVATE ${HDF5_INCLUDE_DIRS})
		target_include_directories(ivs_executable PRIVATE ${HDF5_INCLUDE_DIRS})

		# Link libraries
		target_link_libraries(ivslib ${HDF5_LIBRARIES})
		target_link_libraries(ivs_executable ivslib ${HDF5_LIBRARIES})
		

else()
    # Set the policy to NEW to use <PackageName>_ROOT variables
		cmake_policy(SET CMP0074 NEW)

		# Specify the path to HDF5, otherwise it will go to /opt/hdf5, the cross-compiled hdf5
		set(HDF5_ROOT "/usr")

		# Find HDF5 package (both C and C++ components)
		find_package(HDF5 COMPONENTS C CXX REQUIRED)
    
    # Print the HDF5 variables to verify
    message(STATUS "Compiling for Linux")
    message(STATUS "HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}")
    message(STATUS "HDF5_C_LIBRARIES: ${HDF5_C_LIBRARIES}")
    message(STATUS "HDF5_CXX_LIBRARIES: ${HDF5_CXX_LIBRARIES}")
    
		include_directories(include)
		# Include HDF5 header directories
		include_directories(${HDF5_INCLUDE_DIRS})
		
		# Add the library for your main source
		add_library(ivslib STATIC src/ivs.cpp) # STATIC does not become shared library
		
		# Add the executable for your tests
		add_executable(ivs_executable tests/main.cpp)

    # Link libraries
    # If using serial HDF5
    target_link_libraries(ivslib ${HDF5_C_LIBRARIES})
		target_link_libraries(ivs_executable ivslib ${HDF5_C_LIBRARIES} ${HDF5_CXX_LIBRARIES})
    
endif()





