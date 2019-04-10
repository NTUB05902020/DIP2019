import numpy as np
import cv2
import sys

arr = np.fromfile(sys.argv[1], dtype=np.uint8)
size, name = int(np.sqrt(len(arr))), sys.argv[1][:-3]+'bmp'
arr = np.reshape(arr, (size,size))
cv2.imwrite(name, arr)