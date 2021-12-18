import networkx as nx
import matplotlib.pyplot as plt
import pandas as pd
import os
import sys

def create_graph(file_to_read, num_clients, edge_set, nurse_index=999, is_nurse_list=False):

    fig, ax = plt.subplots()
    # plt.figure()

    # Figure title
    if is_nurse_list: # list of specific nurses, add all indices of nurses to the title
        title_string = str(file_to_read) + '-nurses'  
        for i in range(len(nurse_index)):
            title_string += '-' + str(nurse_index[i])
        plt.title(title_string)
    else:
        if nurse_index == 999: # all nurses
            plt.title(str(file_to_read))
        elif nurse_index < 999:
            plt.title(str(file_to_read) + '-nurse' + str(nurse_index)) # one specific nurse

    # Create graph
    G = nx.MultiGraph()

    # Add nodes to graph
    df = pd.read_csv(r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/ExactMethod/18-4-s-2a_coords.csv') 

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
    # pos = {0: (0,0), 1: (0,1), 2: (0, 2), 3: (0, 3), 4: (0,4), 5: (1,0), 6: (1,1), 7: (1,2), 8: (1,3), 9: (1,4), 10: (2,0), 11: (2,1), 12: (2,2), 13: (2,3), 14: (2,4), 15: (3,0), 16: (3,1), 17: (3,2), 18: (3,3), 19: (3,4) }
    # G.add_nodes_from(range(0,num_clients))
    G.add_nodes_from(pos.keys())

    for i in range(len(edge_set)):
        G.add_edges_from(edge_set[i])

    for edge in G.edges():
        source, target = edge
        rad = 0.2
        arrowprops=dict(arrowstyle="-", 
                        connectionstyle=f"arc3,rad={rad}",
                        linestyle= '-',
                        alpha=0.6)
        ax.annotate("",
                    xy=pos[source],
                    xytext=pos[target],
                    arrowprops=arrowprops
                )
    # nodes
    # nx.draw_networkx_nodes(G, pos, node_size=700, node_color='black')
    # labels
    # nx.draw_networkx_labels(G, pos, font_size=20, font_family="sans-serif", font_color ='white')
    # Create color map for nodes, initial and final depot nodes in red, all other nodes in white.
    node_colour_map = []
    for i in range(num_clients):
        if i == 0 or i == num_clients-1:
            node_colour_map.append('red')
        else:
            node_colour_map.append('white')

    # Draw nodes on graph
    nx.draw_networkx_labels(G, pos, font_size=8)
    nx.draw_networkx_nodes(G, pos, node_color=node_colour_map, node_size=200, edgecolors='black')

    # Add labels to graph

    plt.box(False)
    plt.show()
    exit(-1)
    # for n, p in iter(pos.items()):
        # G.nodes[n]['pos'] = p
    # nx.draw(X, pos)
    # G.add_edges_from(edge_set[0])

    # Layout of graph
    # pos = nx.spring_layout(G)
    # pos = nx.circular_layout(G)
    # pos = nx.planar_layout(G)

    # # Create color map for nodes, initial and final depot nodes in red, all other nodes in white.
    # node_colour_map = []
    # for i in range(num_clients):
    #     if i == 0 or i == num_clients-1:
    #         node_colour_map.append('red')
    #     else:
    #         node_colour_map.append('white')

    # # Draw nodes on graph
    # nx.draw_networkx_nodes(G, pos, node_color=node_colour_map, node_size=200, edgecolors='black')

    # # Add labels to graph
    # nx.draw_networkx_labels(G, pos, font_size=8)

    # # Create colour map for edges, different nurses have different colours
    # edge_colour_map = ['red', 'blue', 'orange', 'green', 'm', 'c', 'k']

    # # Create list of edge types for different nurses, curved edges prevent two edges from different sets from overlapping
    # connection_style = ['arc3,rad=0.1', 'arc3,rad=0.15', 'arc3,rad=0.2', 'arc3,rad=0.25']

    # # Draw edges on graph
    # if is_nurse_list: # multiple nurses given in a list form, subset of nurses
    #     for i in range(len(nurse_index)): # nurse_index here will be a list of nurse indices
    #         nx.draw_networkx_edges(G, pos, connectionstyle=connection_style[i], edgelist=edge_set[nurse_index[i]], alpha=0.5, edge_color=edge_colour_map[i], arrowstyle='-')
    # else:
    #     if nurse_index == 999: # all nurses
    #         for i in range(len(edge_set)):
    #             nx.draw_networkx_edges(G, pos, connectionstyle=connection_style[i], edgelist=edge_set[i],alpha=0.5, edge_color=edge_colour_map[i], arrowstyle='-')
    #     elif nurse_index < 999: # specific nurse
    #         nx.draw_networkx_edges(G, pos, connectionstyle='arc3, rad=0.3', edgelist=edge_set[nurse_index], alpha=0.5, edge_color='r', arrowstyle='-')

    # Information about graph
    print("Total number of nodes: ", int(G.number_of_nodes()))
    print("Total number of edges: ", int(G.number_of_edges()))
    print("List of all nodes: ", list(G.nodes()))
    print("List of all edges: ", list(G.edges(data = True)))
    print("Degree for all nodes: ", dict(G.degree()))
    # print("Total number of self-loops: ", int(G.number_of_selfloops()))
    # print("List of all nodes with self-loops: ", list(G.nodes_with_selfloops()))
    # print("List of all nodes we can go to in a single step from node 2: ", list(G.neighbors(2)))
        
    # Plot the graph
    ax.set_xlim(min_x-2,max_x+2)
    ax.set_ylim(min_y-2,max_y+2)
    ax.tick_params(left=True, bottom=True, labelleft=True, labelbottom=True)
    plt.axis()
    plt.draw()

    # Create filename for the graph
    graph_filename = str(file_to_read)
    if is_nurse_list: # list of specific nurses, add all indices of nurses to the title
        graph_filename += '-nurses'  
        for i in range(len(nurse_index)):
            graph_filename += '-' + str(nurse_index[i])
        graph_filename += '.png'
    else:
        if nurse_index == 999: # all nurses
            graph_filename += '.png'
        elif nurse_index < 999: # single specified nurse
            graph_filename += '-nurse' + str(nurse_index) + '.png'

    # Save graph figure
    # cwd = os.getcwd()
    # outputfiles_path = os.path.join(cwd, 'graphs')
    # outputfile_path1 = os.path.join(outputfiles_path, filename)
    plt.savefig(graph_filename, bbox_inches='tight')
    # plt.show()

    return plt
### --- End def create_graph --- ###