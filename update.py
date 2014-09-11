# Python script for automating the update process from https://github.com/Viddi/Mpi

import os
import sys

os.system('wget https://github.com/Viddi/Mpi/archive/master.zip')
os.system('unzip master.zip')
os.system('rm -rf Mpi-Master/lib/mpich')
os.system('cp -rf Mpi/lib/* Mpi-Master/lib')
os.system('rm -rf Mpi')
os.system('mv Mpi-Master Mpi')

# Clean up
os.system('rm -rf master.zip')

# TOOD: Update library files (exclude binaries)
