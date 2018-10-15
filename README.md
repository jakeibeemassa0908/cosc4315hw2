# cosc4315hw2

A C++ implementation of cosc4315's homework 2.

## Setup

This project uses CMake 3.5 as a build system. To compile the project, run the
following commands:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

The root level CMake config currently builds two entities: libmypython and
test_libmypython. The former is the backbone library for mypython, while the
latter is unit tests for libmypython.

To run the tests for libmpython, you can run the test executable in the test
folder of the compiled binaries.
