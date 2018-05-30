import sys
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
import matplotlib.pyplot as plt
import matplotlib.patches as patches

if len(sys.argv) < 2: 
    print ("Use:", sys.argv[0], "(layout filename)")
    sys.exit(1) 
filename = sys.argv[1]

file = open(filename, "r")
line = file.readline() # read boundary
pos = line.find(';')
line = line[:pos]
offset_x, offset_y, width, height = line.split(' ') 
offset_x = int(offset_x)
offset_y = int(offset_y)
width    = int(width) - offset_x
height   = int(height) - offset_y

if(width > height):
    lims = (0,width)
else:
    lims = (0,height)

fig = plt.figure()

#print(offset_x ,offset_y, width, height)
for line in iter(file):
    pos = line.find("normal")
    line = line[:pos]
    #print(line)
    x, bl_x, bl_y, tr_x, tr_y, net_num, layer, y = line.split(' ')
    print(x, bl_x, bl_y, tr_x, tr_y, net_num, layer, y )
    bl_x = int(bl_x) - offset_x
    bl_y = int(bl_y) - offset_y
    tr_x = int(tr_x) - offset_x - bl_x
    tr_y = int(tr_y) - offset_y - bl_y
    net_num = int(net_num)
    #print(bl_x, bl_y, tr_x, tr_y)
    bl_pt = [bl_x,bl_y]
    ax1 = fig.add_subplot(111, aspect='equal')
    if net_num == 0: # not critical
        ax1.add_patch(patches.Rectangle((bl_x,bl_y),tr_x,tr_y, facecolor = "red"))
    else:
        ax1.add_patch(patches.Rectangle((bl_x,bl_y),tr_x,tr_y, facecolor = "blue"))
    
file.close()
plt.xlim((0,width))
plt.ylim((0,height))

plt.xlabel('X')
plt.ylabel('Y')
plt.show()