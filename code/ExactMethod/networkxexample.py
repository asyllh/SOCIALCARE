import pandas as pd
import networkx as nx
import matplotlib.pyplot as plt

df = pd.DataFrame({'id_emp' : [13524791000109, 12053850000137, 4707821000113, 4707821000114, 1],
           'name_emp': ['Cristiano', 'Gaúcho', 'Fenômeno','Angelin', 'Souza'],
           'name_dep': ['Ronaldo','Ronaldo', 'Ronaldo', 'Ronaldo', 'Bruno'],
           'weight_1': [8,9,10,11,12],
           'weight_2':[5,6,7,8,9] })

G = nx.MultiDiGraph()

G.add_nodes_from(df['id_emp'], bipartite = 0)
emp = [v for v in G.nodes if G.nodes[v]['bipartite'] == 0]
G.add_nodes_from(df['name_dep'], bipartite = 1)
dep = [v for v in G.nodes if G.nodes[v]['bipartite'] == 1]

G.add_weighted_edges_from(df[['name_dep', 'id_emp', 'weight_1']].values)
G.add_weighted_edges_from(df[['id_emp', 'name_dep', 'weight_2']].values)
edge_width = [a[2]['weight']//2 for a in G.edges(data=True)]

plt.figure(figsize=(5,5))

pos = nx.spring_layout(G, k=0.9)

# Here there is the addition:

nx.draw_networkx_nodes(G, pos, nodelist=dep, node_color='#bfbf7f', node_shape="h", node_size=300)
nx.draw_networkx_nodes(G, pos, nodelist=emp, node_color='red', node_size=300)
nx.draw_networkx_edges(G, pos, connectionstyle='arc3, rad = 0.3', width=edge_width, alpha=0.2)

plt.axis('off')
plt.show()