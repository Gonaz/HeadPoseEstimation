import math, Gnuplot

nameFile = "../training/newoutput1"
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

data = list()
for key in values.keys():
	f = open("pitch"+str(key), "wt")
	for val in values[key]:
		if not math.isnan(val):
			f.write(str(val))
			f.write("\t")
			f.write(str(key))
			f.write("\n")
			data.append((val, key))
	f.close()
	print(key, min(values[key]), max(values[key]))

gp = Gnuplot.Gnuplot()
gp.title("Pitch")
#gp('set multiplot')
gp('set xrange [-35:30]')
gp('set yrange [-3:3]')
gp.plot(data)
gp.hardcopy(filename="newoutput1.pdf", terminal="pdf", color=False)
#gp.plot(data)
raw_input("Press enter to continue...")

#from math import isnan

#f = open("../training/mouthOrigP", "rt")
#g = open("../training/positionsPitchOrig", "rt")
#values = {-2:list(), -1:list(), 0:list(), 1:list(), 2:list()}
#counter = 0
#for line1,line2 in zip(f,g):
	#if counter == 0:
		#pass
	#elif counter == 1:
		#index = int(line1)
	#elif counter == 2:
		#values[index].append((float(line1), float(line2)))
	#counter = (counter + 1) % 3

#gp = Gnuplot.Gnuplot()
#gp.title("Visualisation")
#gp("set xrange [-2:2]")
#gp("set yrange [0:2]")
#gp("set multiplot")

#newValues = {-2:list(), -1:list(), 0:list(), 1:list(), 2:list()}
#for key in values.keys():
	#for val in values[key]:
		#if not isnan(val[0]) and not isnan(val[1]):
			#newValues[key].append(val)

#gp.plot(newValues[-2], newValues[-1], newValues[0], newValues[1], newValues[2])
#raw_input("Press enter to continue...")
