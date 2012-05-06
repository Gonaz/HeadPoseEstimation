from pprint import pprint
f = open("../training/positionsYaw")
counter = 0
positions = {'L':list(), 'R':list(), 'N':list()}

def calculatePosition(name):
	if name.count('YR'):
		s = name.split("_")
		return s[-2][0]
	else:
		return 'N'

for line in f:
	if counter == 0:
		position = calculatePosition(line)
		counter += 1
	elif counter == 1:
		val = int(line)
		counter += 1
	elif counter == 2:
		if int(line) == 0:
			positions[position].append(val)
		counter = 0

for key in positions.keys():
	positions[key].sort()
pprint(positions)

