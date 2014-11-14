# Script to update repository on all nodes that are in the machinefile

import os
import sys

nodes = (node.rstrip('\n') for node in open('/home/pi/Development/Mpi/machinefile'))

for node in nodes:
    if node != '10.10.1.236':
        print "Preparing to update pi@" + node

        os.system('ssh pi@' + node + ' mv ~/Development/Mpi/lib ~/Development/')
        os.system('ssh pi@' + node + ' rm -rf ~/Development/Mpi')
        os.system('scp -r Mpi/ pi@' + node + ':/home/pi/Development')
        os.system('ssh pi@' + node + ' rm -rf ~/Development/Mpi/lib')
        os.system('ssh pi@' + node + ' mv ~/Development/lib ~/Development/Mpi')
