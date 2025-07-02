# SWIG usage

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



