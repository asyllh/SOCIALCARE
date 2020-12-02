import sys

# Read quality of last run from file (second word of last line)

file = sys.argv[1]
f = open(file, 'r')
par = f.read()
# print(par)
# print('--')
lines = par.split('\n')
# print(lines)
# print('----')
last_line = lines[-2]
# print(last_line)
words = last_line.split('\t')
print(float(words[1]))
f.close()