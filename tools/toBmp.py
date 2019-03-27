import numpy as np
from PIL import Image
import sys
import os
import struct

last = sys.argv[1] if sys.argv[1].endswith('/') else sys.argv[1]+'/'

for v in os.listdir(last):
	if v.endswith('raw'):
		fName = last+v
		with open(fName, 'rb') as f:
			arr = [[struct.unpack('B', f.read(1))[0] for j in range(256)] for i in range(256)]
			Image.fromarray(np.uint8(arr), 'L').save(fName[:-3]+'bmp', 'BMP')
