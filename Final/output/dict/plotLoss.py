import matplotlib.pyplot as plt
import numpy as np

arr = list()

f = open('loss.txt', 'r')
for line in iter(f):
	arr.append(float(line)/8468)
arr = np.array(arr)

N = len(arr)
print(arr)
print(N)

plt.plot(np.arange(N-5), arr[5:])
plt.savefig('loss.png')
