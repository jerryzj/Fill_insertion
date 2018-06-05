a=[0,0]
b=[0,5]
c=[7,0]
d=[7,5]
width = c[0] - a[0]
height = d[1] - a[1]
lims = (0, 10)

import matplotlib.pyplot as plt
import matplotlib.patches as patches


fig1 = plt.figure()
ax1 = fig1.add_subplot(111, aspect='equal')
ax1.add_patch(
    patches.Rectangle((2, 2), width, height))
plt.ylim(lims)
plt.xlim(lims)
fig1.savefig('auto.png')