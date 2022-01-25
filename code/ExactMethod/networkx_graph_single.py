import networkx as nx
import matplotlib.pyplot as plt
import pandas as pd
import os
import sys

def create_graph(file_to_read, num_clients, edge_set, edge_dicts):

    fig, ax = plt.subplots()
    # plt.figure()

    # Figure title
    title_string = str(file_to_read) + '-nwxs'
    plt.title(title_string)

    # Create graph
    G = nx.MultiDiGraph()

    # Add nodes to graph
    # df = pd.read_csv(r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/ExactMethod/18-4-s-2a_coords.csv')
    df = pd.read_csv(r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/ExactMethod/coordinate_calculations/18-4-s-2a_sym_coords.csv') 

    pos = {}
    client_column = df['n']
    x_column = df['x']
    y_column = df['y']
    min_x = 999
    max_x = 0
    min_y = 999
    max_y = 0


    for i in range(len(x_column)):
        x_coord = df['x'][i]
        y_coord = df['y'][i]
        if x_coord > max_x:
            max_x = x_coord
        elif x_coord < min_x:
            min_x = x_coord
        if y_coord > max_y:
            max_y = y_coord
        elif y_coord < min_y:
            min_y = y_coord
        pos[df['n'][i]] = x_coord, y_coord
    
    print('min_x: ', min_x, ' max_x: ', max_x, ' min_y: ', min_y, ' max_y: ', max_y)
    print('pos.keys(): ', pos.keys())
    # exit(-1)
    # pos = {0: (0,0), 1: (0,1), 2: (0, 2), 3: (0, 3), 4: (0,4), 5: (1,0), 6: (1,1), 7: (1,2), 8: (1,3), 9: (1,4), 10: (2,0), 11: (2,1), 12: (2,2), 13: (2,3), 14: (2,4), 15: (3,0), 16: (3,1), 17: (3,2), 18: (3,3), 19: (3,4) }
    # G.add_nodes_from(range(0,num_clients))
    G.add_nodes_from(pos.keys())
    # for n, p in iter(pos.items()):
        # G.nodes[n]['pos'] = p
    # nx.draw(X, pos)
    # G.add_edges_from(edge_set[0])

    # Create color map for nodes, initial and final depot nodes in red, all other nodes in white.
    node_colour_map = []
    for i in range(num_clients):
        if i == 0 or i == num_clients-1:
            node_colour_map.append('red')
        else:
            node_colour_map.append('white')

    # Draw nodes on graph
    nx.draw_networkx_nodes(G, pos, node_color=node_colour_map, node_size=200, edgecolors='black')

    # Add labels to graph
    nx.draw_networkx_labels(G, pos, font_size=8)

    # Create colour map for edges, different nurses have different colours
    edge_colour_map = ['red', 'blue', 'orange', 'green', 'm', 'c', 'k']

    # Create list of edge types for different nurses, curved edges prevent two edges from different sets from overlapping
    connection_style = ['arc3,rad=0.1', 'arc3,rad=0.15', 'arc3,rad=0.2', 'arc3,rad=0.25']

    # Draw edges on graph
    for i in range(len(edge_set)):
        nx.draw_networkx_edges(G, pos, connectionstyle=connection_style[i], edgelist=edge_set[i], alpha=0.5, edge_color=edge_colour_map[i], arrowstyle='-')

    # nx.draw_networkx_edge_labels(G, pos)
    # nx.draw_networkx_edge_labels(G, pos, edge_labels={(0, 6): '5'}, font_color='red')
    for i in range(len(edge_dicts)):
        nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_dicts[i], font_size=6)

    # Information about graph
    print("Total number of nodes: ", int(G.number_of_nodes()))
    print("Total number of edges: ", int(G.number_of_edges()))
    print("List of all nodes: ", list(G.nodes()))
    print("List of all edges: ", list(G.edges(data = True)))
    print("List of all edges: ", list(G.edges()))
    print("Degree for all nodes: ", dict(G.degree()))
        
    # Plot the graph
    ax.set_xlim(min_x-2,max_x+2)
    ax.set_ylim(min_y-2,max_y+2)
    ax.tick_params(left=True, bottom=True, labelleft=True, labelbottom=True)
    plt.axis()
    plt.draw()

    # Create filename for the graph
    graph_filename = str(file_to_read) + '-nwxs.png'

    # Save graph figure
    plt.savefig(graph_filename, bbox_inches='tight')
    # plt.show()

    return plt
### --- End def create_graph --- ###