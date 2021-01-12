# Build and Install {#build}

## How to Build

`veriblock-pop-cpp` is a CMake project, so it follows all best practices for CMake projects.

To build library:
```sh
# clone repo
$ git clone https://github.com/VeriBlock/alt-integration-cpp
$ cd alt-integration-cpp

# create build dir
$ mkdir build
$ cd build

# build static library in release mode
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DSHARED=OFF 
$ make -j

# install it in your OS globally
$ sudo make install
```

## CMake flags

Legend:
- `{[A],B}` - A or B, default A

Flags:
- `-DTESTING={[ON],OFF}` - do not build tests
- `-DCMAKE_BUILD_TYPE={Release,[Debug]}` - build library in release mode or debug mode
- `-DWITH_PYPOPMINER={ON,[OFF]}` - build python package `pypopminer` - it is a python3 package, which wraps this library, and can be used for [functional](https://github.com/bitcoin/bitcoin/tree/master/test/functional) tests.
   It depends on `python3`, `python3-dev`, `libboost-all-dev` and `libboost-python-dev`
- `-DSHARED={ON,[OFF]}` - build shared library (`.so, .dll, .dylib`) or static library (`.a, .lib`)

Full list of flags can be found at [root CMakeLists.txt#55-70](https://github.com/VeriBlock/alt-integration-cpp/blob/master/CMakeLists.txt#L55-L70).
