![CI](https://github.com/VeriBlock/alt-integration-cpp/workflows/CI/badge.svg)

# alt-integration-cpp

## Dependencies

- (optional) gtest/gmock - https://github.com/google/googletest/releases/tag/release-1.10.0

Run
```
git submodule update --init
```

- (optional) RocksDB - https://github.com/facebook/rocksdb

## Linux

- Run cmake
```bash
cmake . -Bbuild -DFIND_ROCKSDB=OFF
```

## Windows

- Install RocksDB via vcpkg
```bash
vcpkg install rocksdb:x64-windows
```

- Run cmake with vcpkg toolchain
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -A x64 -S . -B build
```

- Open build/alt-integration.sln with VisualStudio
