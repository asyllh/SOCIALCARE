import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv(r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/ExactMethod/18-4-s-2a_coords.csv') 

pos_dict = {}
client_column = df['n']
x_column = df['x']
y_column = df['y']


for i in range(len(x_column)):
    x_coord = df['x'][i]
    y_coord = df['y'][i]
    pos_dict[df['n'][i]] = x_coord, y_coord

print('pos dict: ', pos_dict)

exit(-1)
fig, ax = plt.subplots()
my_scatterplot = ax.scatter(df['x'], df['y'])
N = df['n']
X = df['x']
Y = df['y']
ax.set_xlabel('x')
ax.set_ylabel('y')
for i, txt in enumerate(N):
    ax.annotate(txt, (X[i], Y[i]))
plt.show()
















