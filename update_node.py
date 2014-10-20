# Script to update repository on all nodes that are in the machinefile

import os
import sys

f = open('~/Development/Mpi/machinefile', 'r')
nodes = f.readlines()
f.close

for node in nodes:
    if node != '10.10.1.236'
        os.system('ssh pi@' + node + ' rm -rf ~/Development/Mpi')
        os.system('scp -r Mpi/ pi@' + node + ':/home/pi/Development')
