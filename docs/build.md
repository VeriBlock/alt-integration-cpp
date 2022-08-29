# Build and Install {#build}

## How to Build

`veriblock-pop-cpp` is a CMake project, so it follows best practices for CMake projects.

To build library:
```bash
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
- `-DTESTING={[ON],OFF}` - if ON, tests will be built automatically.
- `-DTESTING_GO={ON,[OFF]}` - if ON, Go tests will run as part of `ctest`.
- `-DWITH_PYPOPTOOLS={ON,[OFF]}` - if ON, pypopminer will be built (depends on python3 and boost-python).
- `-DWITH_C_WRAPPER={ON,[OFF]}` - if ON, C bindings will be included in resulting library.
- `-DWITH_ROCKSDB={ON,[OFF]}` - if ON, rocksdb implementation of a storage will be included (required for C wrapper).
- `-DCMAKE_BUILD_TYPE={Release,[Debug]}` - build library in release mode or debug mode
- `-DSHARED={ON,[OFF]}` - build shared library (`.so, .dll, .dylib`) or static library (`.a, .lib`)

Full list of flags can be found at [root CMakeLists.txt#55-70](https://github.com/VeriBlock/alt-integration-cpp/blob/master/CMakeLists.txt#L55-L70).
