# IVsing


A c++ projet that produces a concise h5 file from a large number of txt files in experiemnts.
--

After performing I-V curves mapping using Bias-Spectroscopy module in Nanonis (scanning probe microscopy controller, SPECS GmbH), users would get a great number of data files in format of txt. 
```dat
Experiment	bias spectroscopy	
Saved Date	27.06.2025 14:44:13	
User		
Date		
X (m)	-4.92857E-9	
Y (m)	-4.92857E-9	
Z (m)	-4.0566E-6	
Z offset (m)	0E+0	
Settling time (s)	100E-6	
Integration time (s)	100E-6	
Z-Ctrl hold	TRUE	
Final Z (m)	N/A	
Start time	27.06.2025 14:44:13	
Filter type	None	
Order		
Cutoff frq		

[DATA]
Bias calc (V)	Current (A)
-1.0100000E+0	-293.21839E-9
...

```

It is convenient to open them by a text editor but not efficient for storing data. This is project aimas to extract numeric data from a large number of I-V curves written by Nanonis software and store them in a popular binary format `h5`. This is much more lightweight and data transfer can be much more expeditious. It takes about 7.8 min for transfering 10,000 data files of 140 MB size are copied to a USB stick at an approximate speed of 300 KB/s. ( less time for transfer rate at ~ 1 MB/s USB -> SSD) 

## Table of Contents

- [Installation](#installation) 
  - [Prerequisites](#prerequisites)
    - [Building the Project](#building-the-project)
      - [Linux](#linux) 
      - [Windows](#windows)
        - WSL
        - Cross-compile with MinGW
- [Contribution](#contribution)
- [License](#license)
- [Keywords](#keywords)

## Prerequisites

- GCC and MinGW (Linux) 
- WSL (Windows)
- CMake (version 3.10 or higher)
- C++17
- HDF5 prebuilt/source code  

## Building the Project

### Linux

0. Install some packages
   
   ```bash
   sudo apt install build-essential cmake gcc g++ hdf5-tools libhdf5-dev
   ```

1. Clone the repository:
   
   ```sh
   git clone https://github.com/mithgil/IVsing.git
   cd IVsing
   ```

2. Create a build directory and navigate to it:
   
   ```sh
   mkdir build
   cd build
   ```

3. Build the project step by step

Building under Linux is quite easy, so let us introduce the easiest way and the tricky version, cross-compiling. 

### Windows

There are a few ways to do this job. To cross-compile on windows is very tricky. 

- WSL
  
  WSL does not allow `wget` to write to c drive or `mv` the downloaded file to somewhere in the c drive. Some commands are not allowed in windows. Compiling using WSL is perhaps the easiest way to do among these methods.
  
  - Install some packages:
    
    ```bash
    sudo apt update
    sudo apt install build-essential cmake gcc g++ hdf5-tools libhdf5-dev
    ```
  
  - Navigate to the project directory
    
    ```bash
    cd /mnt/c/path/to/IVsing/build
    ./ivs_executable
    ```
    
     The prebuilt version from Linux should work. `/mnt/` is the directory system in WSL.      

- Cross-compiler on Linux
  
  - Prerequisites
    
     install 
    
    - cross compiler on linux
      
      ```bash
      sudo apt update
      sudo apt install mingw-w64
      ```
      
       Verification by
      
      ```bash
      which x86_64-w64-mingw32-gcc
      which x86_64-w64-mingw32-g++
      ```
      
       returns 
      
      ```
      /usr/bin/x86_64-w64-mingw32-gcc
      /usr/bin/x86_64-w64-mingw32-g++
      ```
      
       in my linux shell. These will be integrated into the toolchain.cmake file later.
    
    - hdf5 (mingw-w64 compatible) for windows (**False!**)
      
      ```bash
      wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.14/hdf5-1.14.4/bin/windows/hdf5-1.14.4-3-win-vs2022_cl.zip
      ```
      
         and move it to `/opt/hdf5`
      
         But this does not work since vs version of hdf5 is incompatible with `mingw-gcc` on linux
      
      - Cross-compile yourself! (Directly on Linux)
        
        - Donwload the source code of hdf5 from [github website](https://github.com/HDFGroup/hdf5/releases)
          
           Please choose "Surce Code".
        
        - Build with a toolchain file for hdf5 to-be used with cross-compiler
          
          ```bash
          tar -xzvf hdf5-hdf5_1.14.4.3
          cd hdf5-hdf5_1.14.4.3
          ```
          
            and create toolchain file
          
          ```
          cat<<EOF >toolchain.cmake
          # Cross-compilation settings
          set(CMAKE_SYSTEM_NAME Windows)
          set(CMAKE_SYSTEM_VERSION 10.0)
          set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc)
          set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++)
          set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
          set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
          set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
          set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
          EOF
          ```
          
          - use cmake for configuring
            
            ```bash
            cmake -G "Unix Makefiles" \
            -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
            -DCMAKE_INSTALL_PREFIX=/path/to/hdf5/hdf5-1.14 \
            -DBUILD_SHARED_LIBS=ON \
            -DBUILD_STATIC_LIBS=OFF \
            -DHDF5_BUILD_CPP_LIB=ON \
            -DHDF5_BUILD_EXAMPLES=OFF \
            -DHDF5_BUILD_TOOLS=OFF \
            ..
            ```
            
            where `gmake` is used here, since `make` cannot work in my case. And `/path/to/install/hdf5` in my case is `/opt/hdf5-1.14`
          
          - Compile 
            
            ```bash
            make -j4
            ```
            
            This will take a bit of time and get a cup of coffee.   
          
          - Install
            
            ```bash
            make install
            ```
            
            install the built files to the specified installation directory (/path/to/install/hdf5). And my `/path/to/install/hdf5` is `/opt/hdf5-1.14` so administrator privilege is required and please do
            
            ```bash
            sudo make install
            ```
            
            and list the files
            
            ```bash
            cd /opt/hdf5-1.14
            ls 
            # returns bin  cmake  include  lib  share
            ```
            
            Here they are! A cross-compiled hdf5 library on linux is just done. This is cool and fun! 
    
    - Example Usage
      
      Build the project with cross-compiler with the compiled hdf5 (by cross-compiler)
      
      ```bash
      cd build
      cmake -DCMAKE_TOOLCHAIN_FILE=../x86_64-w64-mingw32.toolchain.cmake ..
      make
      ```
    
    Finally, we arrive here. Well done. It is worth celebrating if you work out this as well. 
    
    Users can choose the best way to cross-compile and run the binary code on windows. 

- Do CMake!
  
  - Linux
    
    ```bash
    cmake -B build-linux -S ..
    # make files will be under build-linux
    cd build-linux
    make
    ```
  
  - Windows
    
    ```bash
    cmake -B build-windows -DCMAKE_TOOLCHAIN_FILE=../../x86_64-w64-mingw32.toolchain.cmake ..
    cd build-windows
    make
    ```
    
     where `-B` will be the subdirectory under original `build` direcotry.
    
     **NOTE** that the `-DCMAKE_TOOLCHAIN_FILE` will need a grandparent directory `../../` prepended the toolchain file name. Otherwise, cmake will never get the cross-compiling info. 

- Migrate from Linux to Windows
  
   Now you have the executable in the build-windows directory, you don't need to modify the directory structure. However, for better organization and easier access to dependencies, you might want to structure your project directory more clearly. Here's a suggested approach:

   **Recommended Directory Structure on Windows**

      ```vbnet
      project/
      ├── bin/       # Your compiled executables
      ├── lib/       # Additional libraries (HDF5 DLLs)
      ├── include/   # Header files (if needed for development)
      ├── hdf5/      # HDF5 library files
            ├── lib/
            └── include/
      ├── build/     # Build directory for CMake
      ├── src/       # Source files
            └── ivs.cpp
      ├── include/   # Header files
            └── ivs.h
      ├── tests/     # Test files
            └── main.cpp
      ├── CMakeLists.txt
      └── README.md
      ```

   **Steps to Prepare and Run Your Executable**

1. Move Your Executable:
   
   Move the executable from `build-windows` to `bin`:
   
   ```cmd
   move C:\path\to\project\build-windows\your_executable.exe C:\path\to\project\bin
   ```

2. Move HDF5 Library Files:
   
   Copy the HDF5 library files to `hdf5\lib` and `hdf5\include` respectively:
   
   ```cmd
   xcopy C:\path\to\prebuilt\hdf5\lib\* C:\path\to\project\hdf5\lib
   xcopy C:\path\to\prebuilt\hdf5\include\* C:\path\to\project\hdf5\include
   ```

3. Set Environment Variables:
   
   Add the HDF5 lib directory to the PATH environment variable:
   
   ```cmd
   set PATH=C:\path\to\project\hdf5\lib;%PATH%
   
   echo %PATH%
   ```

4. Ensure DLLs are Accessible:
   
   Copy any required DLLs (e.g., MinGW runtime DLLs) to the bin directory if they are not already included.
   
   Eventually, this way of doing does not work perhaps due to `.dll` linking. (I stuck here)
   
   ### Main Usage and Running Your Application

5. Path editing 
   
   Example code snippet in `/tests/main.cpp`
   
   (The executable for linux should be done in the previous section)
   
   - Build for Linux users,
     
     ```c++
        //the directory of the data 
        std::filesystem::path input_directory = "/path/to/your/data";
        // Append the file name to the directory path
        std::filesystem::path output_file = input_directory / "ivmap.h5";
     ```
   
   - Build for Windows users, some modifications in the `main.cpp` file is necessary.
     
     ```c++
        //the directory of the data 
        std::filesystem::path input_directory = "/path/to/your/data";
        // Append the file name to the directory path
        std::filesystem::path output_file = input_directory / "ivmap.h5";
     ```
   
   After the `main.cpp` file modifications, please do `make` again on linux.

6. Run Application
   
   Run executable on linux is neglected here. Now, you can run your executable from the bin directory:
   
   ```cmd
   cd C:\path\to\project\bin
   your_executable.exe
   ```
   
   Example of Handling DLL Dependencies
   If your application requires additional DLLs (such as MinGW DLLs or other dependencies), ensure these DLLs are either in the bin directory or in a directory included in the PATH environment variable. You can find the required DLLs usually in the MinGW bin directory on your Linux machine.

### Supplementary: CMake Info

In this section, some readers can acquire better knowledge about how `cmake` works. If you are not interested in this part, just skip this section.

In a CMake project, target_link_libraries is used to specify which libraries should be linked to a target during the linking stage of the build process. Here's what this means in detail:

A.  C++ project Building Stages
When you build a C++ project, it generally goes through two main stages:

- **Compilation Stage**: Each source file (.cpp or .c file) is compiled independently into an object file (.o or .obj file). During this stage, the compiler needs to know where to find the header files (.h or .hpp files) included in the source files, which is where target_include_directories comes into play.

- **Linking Stage**: All the object files are linked together to produce the final executable or library. During this stage, the linker needs to know which libraries to link against to resolve symbols (functions, variables) that are declared in the headers but defined in other libraries.

##### `target_link_libraries`:

The `target_link_libraries` command tells CMake which libraries are needed to resolve these symbols during the linking stage. Here’s a breakdown of how it works:

- **Static Libraries**: If your project has a static library (`libmy_library.a` or `libmy_library.lib`), `target_link_libraries` will include this static library in the linking process for the executable. The static library’s code is copied into the executable.

- **Shared Libraries**: If you are linking against shared libraries (`libmy_library.so` or `my_library.dll`), `target_link_libraries` will tell the linker to include references to these shared libraries in the final executable. The executable will then depend on these shared libraries at runtime.

B. Example Explanation

```cmake
# Add the library and executable
add_library(my_library STATIC src/my_library.cpp)
add_executable(my_executable src/main.cpp)

# Link libraries for the library and executable
target_link_libraries(my_library PRIVATE ${HDF5_LIBRARIES})
target_link_libraries(my_executable PRIVATE my_library ${HDF5_LIBRARIES})
```

- `add_library(my_library STATIC src/my_library.cpp)`: This command creates a static library named `my_library` from `src/my_library.cpp`.

- `add_executable(my_executable src/main.cpp)`: This command creates an executable named `my_executable` from `src/main.cpp`.

- `target_link_libraries(my_library PRIVATE ${HDF5_LIBRARIES})`:
   -This command links the HDF5_LIBRARIES to the my_library target.
  `${HDF5_LIBRARIES}` is a variable that contains the list of HDF5 library files needed for linking.
  PRIVATE means that these libraries are linked only when building my_library, and not propagated to targets that link against my_library.

- `target_link_libraries(my_executable PRIVATE my_library ${HDF5_LIBRARIES})`:
  
  - This command links both my_library and HDF5_LIBRARIES to the my_executable target.
  - This ensures that when my_executable is being linked, it includes the code from my_library and also links against the HDF5 libraries.
  - PRIVATE means that these libraries are linked only when building my_executable, and not propagated further.

C. What Happens During the Linking Stage
When the linker is invoked for my_executable, it will:

1. Include the object files from my_library.
2. Link against the HDF5 libraries specified in HDF5_LIBRARIES to resolve any HDF5-related symbols used in my_library and my_executable.

This ensures that all necessary code and libraries are included in the final executable, resolving any undefined references to symbols that are declared but not defined within the project’s own source files.

In summary, `target_link_libraries` ensures that the linker includes the specified libraries so that all symbols required by your executable or library are correctly resolved, allowing the final build product to function as expected.

## Contribution

Contributions are welcome! Please fork the repository and submit a pull request. 

## License

Distributed under the MIT general public License. See LICENSE for more information.

## Keywords

- IV-curves
- cmake
- mingw-gcc
- Cross-compile
- `hdf5`
