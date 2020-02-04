# alt-integration-cpp

## Dependencies

- (optional) gtest/gmock - https://github.com/google/googletest/releases/tag/release-1.10.0

- RocksDB - https://github.com/facebook/rocksdb

## Linux

- Install RocksDB as a shared library
```bash
sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev libzstd-dev
git clone https://github.com/facebook/rocksdb.git
cd rocksdb
DEBUG_LEVEL=0 make shared_lib
sudo make install
```

## Windows

- Install gtest via vcpkg [https://github.com/Microsoft/vcpkg]
```bash
vcpkg install gtest:x64-windows
```

- Install RocksDB via vcpkg
```bash
vcpkg install rocksdb:x64-windows
```

- Run cmake with vcpkg toolchain
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -A x64 -S . -B build
```

- Open build/alt-integration.sln with VisualStudio