# Python script for automating the update process from https://github.com/Viddi/Mpi

import os
import sys

os.system('wget https://github.com/Viddi/Mpi/archive/master.zip')

if os.path.exists('Mpi-master.zip')
    os.system('unzip master.zip')
    os.system('rm -rf Mpi-master/lib/mpich')
    os.system('cp -rf Mpi/lib/* Mpi-master/lib')
    os.system('rm -rf Mpi')
    os.system('mv Mpi-master Mpi')
    
    # Clean up
    os.system('rm -rf master.zip')
