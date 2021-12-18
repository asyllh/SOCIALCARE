import networkx as nx
import numpy as np 
import matplotlib.pyplot as plt

fig, ax = plt.subplots()

pos = {0: (4, 2), 1: (2, 3), 2: (1, 3), 3: (3, 1)} 
X=nx.Graph()

X.add_nodes_from(pos.keys())
for n, p in iter(pos.items()):
    X.nodes[n]['pos'] = p
nx.draw(X, pos)

ax.set_xlim(0,5)
ax.set_ylim(0,5)
ax.tick_params(left=True, bottom=True, labelleft=True, labelbottom=True)
plt.axis('on')
plt.show()

# nx.draw_networkx_nodes(X,pos,node_size=3000,nodelist=[0,1,2,3],node_color='r')