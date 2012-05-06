f = open("../training/positionsPitchOrig", "rt")
values = {-2:list(), -1:list(), 0:list(), 1:list(), 2:list()}
counter = 0
for line in f:
	if counter == 0:
		counter += 1
	elif counter == 1:
		index = int(line)
		counter += 1
	elif counter == 2:
		values[index].append(float(line))
		counter = 0
f.close()

minimum = 100
maximum = -100
for key in values.keys():
	f = open("pitch"+str(key), "wt")
	for val in values[key]:
		f.write(str(val))
		f.write("\n")
	f.close()
	print(key, min(values[key]), max(values[key]))
	if min(values[key]) < minimum:
		minimum = min(values[key])
	if max(values[key]) > maximum:
		maximum = max(values[key])

distance = maximum - minimum
stretch = 150

sortedKeys = list(values.keys())
sortedKeys.sort()
for key in sortedKeys:
	previousPosition = 0
	values[key].sort()
	print(str(key)+": ", end = "\t")
	for val in values[key]:
		position = (val-minimum)/distance*stretch
		for i in range(previousPosition, int(position)):
			print(".", end="")
		print("X", end="")
		previousPosition = int(position)
	for i in range(previousPosition, stretch):
		print(".",end="")
	print()

