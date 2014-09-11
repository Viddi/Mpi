# Python script for automating the update process from https://github.com/Viddi/Mpi

import os
import sys

os.system('cd ..')
os.system('wget https://github.com/Viddi/Mpi/archive/master.zip')
os.system('unzip master.zip')
os.system('rm -rf Mpi/src')
os.system('mkdir Mpi/src')
os.system('cp Mpi-Master/src/* Mpi/src/')

# TOOD: Update library files (exclude binaries)
