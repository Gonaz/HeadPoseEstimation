information = dict()
files = list()

def serialize():
	f = open("../training/gmTest2", "wt")
	for elem in files:
		f.write(elem)
		f.write("\n")
		f.write(str(information[elem][0]))
		f.write("\n")
		if elem.count("D") > 0 or elem.count("SD") > 0:
			f.write(str(information[elem][1]*-1))
		else:
			f.write(str(information[elem][1]))
		f.write("\n")
	f.close()

f = open("../training/gmTest", "rt")
counter = 0
maximum = 0
for line in f:
	if counter == 0:
		filename = line.strip()
		files.append(filename)
		information[filename] = list()
	elif counter == 1:
		information[filename].append(int(line))
	elif counter == 2:
		value = float(line)
		if value > maximum:
			maximum = value
		information[filename].append(float(line))
	counter = (counter + 1) % 3
f.close()

for key in information.keys():
	information[key][1] /= maximum

serialize()
