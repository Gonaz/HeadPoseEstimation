import numpy

f = open("../training/mouth", "rt")
files = list()
information = dict()

counter = 0

for line in f:
	line = line.strip()
	if counter == 0:
		x = list()
		y = list()
		fileName = line
		information[fileName] = list()
		files.append(fileName)
	elif counter == 1:
		information[fileName].append(line)
	else:
		split = line.split(",")
		x.append(float(split[0]))
		y.append(float(split[1]))
	counter = (counter + 1) % 5
	if counter == 0:
		coeffs = numpy.polyfit(x, y, deg=2)
		xf = [float(i)/10 for i in range(0, 100)]
		yf = list()
		yf[:] = numpy.polyval(numpy.poly1d(coeffs), xf)

		maxY = max(yf)
		baseY = yf[0]
		#information[fileName].append(maxY/baseY)
		information[fileName].append(coeffs[0]*100)
f.close()

f = open("../training/mouthP", "wt")
for fileName in files:
	f.write(fileName)
	f.write("\n")
	f.write(information[fileName][0])
	f.write("\n")
	f.write(str(information[fileName][1]))
	f.write("\n")
f.close()
