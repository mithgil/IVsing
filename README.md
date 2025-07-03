# IVpy: A Python interface to IVsing of c++ processing

After performing I-V curves mapping using Bias-Spectroscopy module in Nanonis (scanning probe microscopy controller, SPECS GmbH), users would get a great number of data files in format of txt.

This is a natural progression from former c++ project IVsing that converts a myriad of dat/txt files in Bias-Sepctroscopy experiments data into a structured h5.

By providing a Python interface to process Bias-Spectroscopy map data files, user can platform-independent and easily convert irregular, messy data into a structured one.

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

It is convenient to open them by a text editor but not efficient for storing data. This is project aimas to extract numeric data from a large number of I-V curves written by Nanonis software and store them in a popular binary format `h5`. This is much more lightweight and data transfer can be much more expeditious. 


## Table of Contents

  - [Usage]
    - [Building the Project](#building-the-project)
      - [Prerequisites](#prerequisites)
        - [Linux](#linux) 
        - [Windows](#windows)
      - [SWIG]
- [Contribution](#contribution)
- [License](#license)
- [Keywords](#keywords)

## Usage

```python

import datparser

datparser.process_directories("/home/yourname/path/to/data")

```
And it's done pretty much similar to c++

```bash
cd .../IVsing/build
ivs_executable /home/yourname/path/to/data

```
And it will process all the .dat files under the subdirectories without any user inputs and output:

```bash

Subdirectories in "/home/yourname/path/to/data":
--------------------------------------------------
  Processing subdirectory: map2_120by80_600nm_400nm
  - Warning: It looks like your map is not a square. Process breaks
--------------------------------------------------
  Processing subdirectory: map1_300nm
	- Scan pixels: (100, 100)
	- Full Scan range (m): (3e-07, 3e-07)
	- Pixel size (m):3e-09
	- Bias-Spectroscopy: Current-forward only
  -------  
   JSON file successfully written to: "/home/yourname/path/to/data/map1_300nm/ivmapsing.json"
  -------  
   HDF5 file successfully written to: "/home/yourname/path/to/data/map1_300nm/ivmapsing.h5"
    HDF5 file structured as:
      /ExpDate
      /scanrange/
      ├──unit
      ├──x
      └──y
      /scanpixels/
      ├──x
      └──y
      /datasets/
      ├──z (optional)
      ├──bias
      └──current_fwd

```
This will nevigates to each subdirectory of your given root directory.

Congrats! You will see json and h5 in your subdirectory for further processing. 


## Building the Project

### Prerequisites (c++)

- GCC and MinGW (Linux) 
- WSL (Windows)
- CMake (version 3.10 or higher)
- C++17
- HDF5 prebuilt/source code  


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

## SWIG usage

```
~/c/ivsing
├── include/
│   ├── ivs.h
│   ├── utilities.h
│   └── json.hpp
├── src/
│   ├── ivs.cpp
│   ├── utilities.cpp
├── datparser.i
```

✅ SWIG – generate wrapper
From the project root (~/c/ivsing), run:

```bash
swig -c++ -python -Iinclude datparser.i
```
This creates:

datparser_wrap.cxx — the C++ wrapper

datparser.py — the Python interface stub

✅ check installed HDF5 and compile
```
ls /usr/include/hdf5/serial/H5Cpp.h
```

✅ Compile SWIG wrapper
```bash

g++ -std=c++20 -fPIC \
    -Iinclude \
    -I/usr/include/python3.10 \
    -I/usr/include/hdf5/serial \
    -c datparser_wrap.cxx -o datparser_wrap.o
    
```

You’ll also need that -I/usr/include/hdf5/serial when compiling your .cpp files.


✅ Compile your source files
```
g++ -std=c++20 -fPIC -Iinclude -I/usr/include/hdf5/serial -c src/ivs.cpp -o ivs.o
g++ -std=c++20 -fPIC -Iinclude -I/usr/include/hdf5/serial -c src/utilities.cpp -o utilities.o
```


✅ Link everything into Python module

```
g++ -shared datparser_wrap.o ivs.o utilities.o \
    -L/usr/lib/x86_64-linux-gnu/hdf5/serial \
    -lhdf5_cpp -lhdf5 \
    -o _datparser.so

```

Then

```bash
python3 -c "import datparser; print(dir(datparser))"
```

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
