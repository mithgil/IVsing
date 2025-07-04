# IVpy: A Python interface to IVsing of c++ processing


After performing I-V curves mapping using Bias-Spectroscopy module in Nanonis (scanning probe microscopy controller, SPECS GmbH), users would get a great number of data files in format of txt.

This is a natural progression from former c++ project IVsing that converts a myriad of dat/txt files in Bias-Sepctroscopy experiments data into a structured h5.

By providing a Python interface to process Bias-Spectroscopy map data files, user can platform-independent and easily convert irregular, messy data into a structured one.


--


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


## Table of Contents

  - [Usage](#Usage)
    - [Building the Project](#building-the-project)
      - [Prerequisites](#prerequisites)
        - [Linux](#linux) 
        - [Windows](#windows)
      - [SWIG](#SWIG)
- [Contribution](#contribution)
- [License](#license)
- [Keywords](#keywords)

## Usage



Download the package

```bash
 git clone https://github.com/mithgil/IVsing.git0
```

Copy `datparser.py` and `_datparser.so` to the root directory of your project, then


With `datparser.py` and `_datparser.so` files (Linux), 

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

Congrats! You will see json and h5 in your subdirectory for further processing. 

## Visualization of Map

If the dat files are not processed yet, go to the earlier stage to do processing.

With `h5utils.py`, you can view the structure of the imported h5 easily and access the datasets inside it.

```python

import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
mpl.rcParams['font.family'] = 'sans-serif'
mpl.rcParams['font.sans-serif'] = ['Arial']
import h5py
import os

# from module
from h5utils import HDF5Analyzer

subpath = "map1" # replace with your defined name

h5_file_path = os.path.join(root_path, subpath, "ivmapsing.h5")
analyzer = HDF5Analyzer(h5_file_path)
analyzer.list_h5_struct()

```
it will out put

```bash

--- Listing all groups and datasets in '/home/yourname/path/to/data//map1_300nm/ivmapsing.h5' by traversing the hierarchy ---
GROUP: /data/
    DATASET: /data/bias (Shape: (504,), Dtype: float64)
    DATASET: /data/current_fwd (Shape: (504, 10000), Dtype: float64)
GROUP: /scanpixels/
    DATASET: /scanpixels/values (Shape: (2,), Dtype: int32)
GROUP: /scanrange/
  - ATTRIBUTE: 'unit' = 'b'm''
    DATASET: /scanrange/values (Shape: (2,), Dtype: float64)
```
```python
analyzer.build_struct_dict()
analyzer.h5struct
```

This will show the whole structure of the h5 file

```bash

--- Building a Python dictionary from HDF5 file '/home/yourname/path/to/data//map1_300nm/ivmapsing.h5' ---

Successfully built the structured dictionary. File is now closed.
An error occurred: name 'h5_structure_dict' is not defined
{'_attributes': {'ExpDate': b'03.06.2025'},
 'data': {'bias': {'type': 'dataset',
   'shape': (504,),
   'dtype': 'float64',
   'path': '/data/bias',
   'data': array([-4.0400001e-01, -4.0240160e-01, -4.0080321e-01, -3.9920479e-01,
          -3.9760637e-01, -3.9600796e-01, -3.9440957e-01, -3.9281115e-01,
...

 'scanpixels': {'values': {'type': 'dataset',
   'shape': (2,),
   'dtype': 'int32',
   'path': '/scanpixels/values',
   'data': array([100, 100], dtype=int32)}},
 'scanrange': {'_attributes': {'unit': b'm'},
  'values': {'type': 'dataset',
   'shape': (2,),
   'dtype': 'float64',
   'path': '/scanrange/values',
   'data': array([3.e-07, 3.e-07])}}}


```

```python

bias = analyzer.h5struct['data']['bias']['data']
current_map = analyzer.h5struct['data']['current_fwd']['data']
unit = analyzer.h5struct['scanrange']['_attributes']['unit'].decode('utf-8')
pixels = analyzer.h5struct['scanpixels']['values']['data']

# matplotlib
fig, ax = plt.subplots(1,2, figsize = (17,6))

bias_slice = 0.1
index = analyzer.get_nearest_index(bias_slice)

x = np.arange(pixels[0])
y = np.arange(pixels[1])
X, Y = np.meshgrid(x, y)

data = np.reshape(current_map[index,:] *1e6, (pixels[0], pixels[1]))

heatmap = ax[0].imshow(data,
                    aspect='equal',
                    origin='lower',
                    cmap = 'Blues_r')

cbar = fig.colorbar(heatmap, ax=ax[0], orientation='horizontal',
                    pad=0,
                    aspect = 9,
                    shrink=0.2,
                    anchor=(0.35, 0.6)
                   )

cbar.ax.tick_params(labelsize=16)

x0, y0 = 10, 25

ax[0].scatter(x0, y0,
            c = 'red',
            s=50,                 
            marker='o',             
            edgecolors='black',     
            alpha=0.8,              
            label='')

position_index = analyzer.get_position_index(x0, y0)

ax[0].set_xticks([]) 
ax[0].set_yticks([]) 
ax[0].set_title(f"Bias = {bias_slice} V", fontsize = 18)

ax[1].plot(bias[3:], current_map[3:,position_index], lw = 2)

ax[1].set_xlabel("Bias (V)", fontsize = 22)
ax[1].set_ylabel("Current (A)", fontsize = 22)
ax[1].yaxis.get_offset_text().set_fontsize(15) # order on top of y axis

ax[1].tick_params(axis='both', labelsize=16) 
    
output_filename = h5_file_path[0:-3] + "_map_curves_matplot.png"
print(output_filename)

plt.tight_layout()
plt.savefig(output_filename, dpi=500, bbox_inches = 'tight')
plt.show()

```

This will show the map data on the left and I-V curve on the right. What a nice visualization tool!


## Building the Project

### Prerequisites (c++)

- GCC and MinGW (Linux) 
- WSL (Windows)
- CMake (version 3.10 or higher)
- C++17
- HDF5 prebuilt/source code  


### Linux

0. Install prerequisites
   
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
    
  - Install some packages as the previous section
    

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
- Nanonis
- SWIG
- Python
- I-V curves
- cmake
- hdf5
