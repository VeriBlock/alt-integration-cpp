# alt-integration-cpp

## Dependencies

- (optional) gtest/gmock - https://github.com/google/googletest/releases/tag/release-1.10.0

## Windows

- Install gtest via vcpkg [https://github.com/Microsoft/vcpkg]
```bash
vcpkg install gtest:x64-windows
```

- Run cmake with vcpkg toolchain
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -A x64 -S . -B build
```

- Open build/alt-integration.sln with VisualStudio