import numpy as np
from PIL import Image
import sys
import os
import struct

fileName = sys.argv[1]
with open(fileName, 'rb') as f:
	
	R = [[struct.unpack('B', f.read(1))[0] for j in range(256)] for i in range(256)]
	G = [[struct.unpack('B', f.read(1))[0] for j in range(256)] for i in range(256)]
	B = [[struct.unpack('B', f.read(1))[0] for j in range(256)] for i in range(256)]
	arr = list()
	for i in range(256):
		row = list()
		for j in range(256):
			row.append([R[i][j], G[i][j], B[i][j]])
		arr.append(row)
	Image.fromarray(np.uint8(arr), 'RGB').save(fileName[:-3]+'bmp', 'BMP')
