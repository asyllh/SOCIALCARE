import sys

# Read quality of last run from file (second word of last line)

file = sys.argv[1]
f = open(file, 'r')
par = f.read()
f.close()

# print('--')
lines = par.split('\n')
# print(lines)
# print('----')
try:
	last_line = lines[-2]
	words = last_line.split('\t')
	print(float(words[1]))
except:
	print('ERROR: Unable to read "' + str(file) + '"')
	print('File contents were:"\n' + str(par) + '\n"')
	if len(sys.argv) > 2:
		file2 = sys.argv[2]
		print('Error file provided: "' + str(file2) + '"\nContents:')
		f2 = open(file2, 'r')
		par2 = f2.read()
		print(par2)
		print('^^^^^^^^^')
	print('end of error')