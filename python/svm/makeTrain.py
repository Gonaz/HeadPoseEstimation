f = open("../../training/positionsPitchNew", "rt")

def p(string):
	print(string, end="")

def prettyPrint(elements):
	for key in elements.keys():
		p(key)
		p(" ")
		for i in range(0, len(elements[key])):
			p(i)
			p(":")
			p(elements[key][i])
			p(" ")
		p("\n")

def setNumber(path):
	l = path.split("/i")

information = {-2:list(), -1:list(), 0:list(), 1:list(), 2:list()}
counter = 0
for line in f:
	if counter == 0:
		print(line, end="")
		pass
	elif counter == 1:
		index = int(line)
	elif counter == 2:
		value = float(line)
		information[index].append(value)
	counter = (counter + 1)%3
prettyPrint(information)
