# Python script for automating the update process from https://github.com/Viddi/Mpi

import os
import sys

os.system('wget -O latest.zip https://github.com/Viddi/Mpi/archive/latest.zip')

if os.path.exists('latest.zip'):
    os.system('unzip latest.zip')
    os.system('rm -rf Mpi-master/lib/mpich')
    os.system('cp -r Mpi/lib/* Mpi-master/lib')
    os.system('rm -rf Mpi')
    os.system('mv Mpi-master Mpi')
    
    # Clean up
    os.system('rm -rf latest.zip')
