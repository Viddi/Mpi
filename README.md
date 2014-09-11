Mpi
===

Project for Computer Science Research class.

## Installation Guide

**NOTE: This guide is for Linux/OS X only.**

*Download latest version of MPICH [here](http://mpich.org/downloads/).*

### Compiling MPICH 3.1.2

1. `cd /path/to/project && mkdir tmp && cd tmp`
2. `/path/to/mpich-3.1.2/configure --prefix=/path/to/project/lib/mpich --disable-fortran CXXFLAGS="-Wall -g -std=c++11" 2>&1 | tee c.txt`
3. `make 2>&1 | tee m.txt`
4. `make install 2>&1 | tee mi.txt`
5. `rm -rf /path/to/project/tmp`

*This installation guide was referenced from [mpich.org](http://mpich.org/static/downloads/3.1.2/mpich-3.1.2-installguide.pdf).*

### Building Project

*NOTE: Before being able to build the project, you need to make sure you have C++ 11 set up.*

1. `cd /path/to/project/src`
2. `mpicxx main.cc` (A PATH variable needs to be set in your .bashrc file to point to /path/to/project/lib/mpich/bin)
3. `mpiexec ./a.out`

### Pi Node Setup

Set up steps and a download link to a .img file with a pre-configured OS will be available soon.
