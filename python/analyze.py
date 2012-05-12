nameFile = "../training/positionsPitchNew"
print(nameFile)
f = open(nameFile, "rt")
values = {-2:list(), -1:list(), 0:list(), 1:list(), 2:list()}
counter = 0
for line in f:
	if counter == 0:
		counter += 1
	elif counter == 1:
		index = int(line)
		counter += 1
	elif counter == 2:
		if float(line) != 0:
			values[index].append(float(line))
		counter = 0
f.close()

for key in values.keys():
	f = open("pitch"+str(key), "wt")
	for val in values[key]:
		f.write(str(val))
		f.write("\n")
	f.close()
	print(key, min(values[key]), max(values[key]))
