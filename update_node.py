# Script to update repository on all nodes that are in the machinefile

import os
import sys

f = open('machinefile', 'r')
nodes = f.readlines()
f.close

for node in nodes:
    os.system('ssh pi@' + node + ' python ~/Development/update.py')
