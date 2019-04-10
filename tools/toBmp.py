import numpy as np
from PIL import Image
import sys
import os

last = sys.argv[1] if sys.argv[1].endswith('/') else sys.argv[1]+'/'

for v in os.listdir(last):
	if v.endswith('raw'):
		fName = last+v
		arr = np.fromfile(fName, dtype=np.uint8)
		if(len(arr) % 3 == 0): continue
		size = int(np.sqrt(len(arr)))
		arr = np.reshape(arr, (size,size))
		Image.fromarray(arr, 'L').save(fName[:-3]+'bmp', 'BMP')