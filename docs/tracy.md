# How to trace performance of ALT-CPP

See Tracy Documentation: https://github.com/wolfpld/tracy/releases/latest/download/tracy.pdf

## Install TracyServer

1. git clone https://github.com/wolfpld/tracy
2. cd profiler/build/unix
3. Install capstone library and glfw:
   - On Manjaro/Arch: `pamac install capstone glfw-x11`
4. make -j
5. sudo mv Tracy-release /usr/local/bin/TracyServer

## Usage in C++

1. #include <veriblock/pop/trace.hpp>
2. At the beginning of your function use VBK_TRACE_ZONE_SCOPED or VBK_TRACE_ZONE_SCOPED_N (named):
   ```C++
   void foo() {
    VBK_TRACE_ZONE_SCOPED;
   }
   
   void bar() {
    VBK_TRACE_ZONE_SCOPED_N("MyBarFunction");
   }
   ```
3. Run CMake with `-DWITH_TRACER=ON`
4. Compile and run the code. Code can be installed to system, and used in vBTC without changes.

## Run server

First, run `TracyServer` and press "Connect".

Then, run your code. 

