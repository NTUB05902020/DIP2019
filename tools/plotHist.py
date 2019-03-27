import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import sys
import os
import struct

last = sys.argv[1] if sys.argv[1].endswith('/') else sys.argv[1]+'/'

for v in os.listdir(last):
	if v.endswith('hist'):
		print(v)
		fName = last+v
		print(fName)
		with open(fName, 'r') as f:
			arr = list()
			for i in range(256*256):
				arr.append(float(f.readline().strip('\n')))
			plt.clf()
			plt.hist(np.array(arr), bins=50)
			plt.savefig(fName[:-4]+'.png')
