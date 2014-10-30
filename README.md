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
2. `mpicxx -o nova main.cc`
3. `mpiexec ./nova`

### Pi Node Setup

To set up static IP address for each node, replace everything in the *interfaces* file with the contents below. You will have to change the values to fit your own network. As for our cluster, just changing the XX values with the number of the Pi node will be enough.

`sudo vi /etc/network/interfaces`

```
auto lo eth0
iface lo inet loopback
iface eth0 inet static
        address 10.10.53.1XX
        netmask 255.255.0.0
        gateway 10.10.0.254
        dns-nameservers 10.90.0.50 10.90.0.48
```

It's probably a safe bet to reboot the Pi after doing these configurations. Next you will have to generate a new ssh key to allow the master node to communicate with the current node, freely.

1. `ssh-keygen` Overwrite the existing one if it asks for it. Otherwise, just press enter for every prompt it asks.
2. `cd ~/.ssh`
3. `cp id_rsa.pub pi-N` Replace N with the number for the Pi node.
4. Copy the pi-00 file to the ~/.ssh/ directory.
5. `cat pi-00 >> authorized_keys`

Compiling the test prime program:

1. `cd /path/to/project/src`
2. `mpicxx -o prime prime_mpi.cpp`
3. `mpiexec ./prime` Run it to check if it compiled correctly.


---

### TODO
* ~~Write a script to update project on all nodes, from master.~~
* Set up steps and a download link to a .img file with a pre-configured Raspbian OS will be available soon.

