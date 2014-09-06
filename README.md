Mpi
===

Project for Computer Science Research class.

## Installation Guide

*Download latest version of MPICH [here](http://mpich.org/downloads/).*

#### Compiling MPICH 3.1.2

1. `cd /path/to/project && mkdir tmp && cd tmp`
2. `/path/to/mpich-3.1.2/configure --prefix=/path/to/project/lib/mpich --disable-fortran 2>&1 | tee c.txt`
3. `make 2>&1 | tee m.txt`
4. `make install 2>&1 | tee mi.txt`
5. `rm -rf /path/to/project/tmp`

*This installation guide was referenced from [mpich.org](http://mpich.org/static/downloads/3.1.2/mpich-3.1.2-installguide.pdf).*

#### Building Project

1. `cd /path/to/project`
2. `mkdir build && mkdir build/debug`
2. `cd build/debug`
3. `cmake ../.. -DCMAKE_BUILD_TYPE=debug`
4. `make`
5. `./Mpi_d`

**NOTE: This guide is for Linux/OS X only.**

#### Pi Node Setup

1. After booting it up, check Raspbian and install it. The Pi will reboot automatically
2. Select advanced options and change the hostname to 'pi_n'
3. Click finish
