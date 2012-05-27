filename = 'gmTest'
pitchDict = {'U':2, 'SU':1, 'SD':-1, 'D':-2}
information = dict()
files = list()

def mean(_list):
	return sum(_list)/len(_list)	

def serialize():
	g = open(filename+'Cal', 'wt')
	for key in files:
		g.write(key)
		g.write(str(information[key][0]) + "\n")
		g.write(str(information[key][1]) + "\n")
	g.close()
	
def detectPitch(name):
	if name.count('PR') > 0:
		s = name.split('_')
		return pitchDict[s[-2]]
	else:
		return 0

def detectImage(name):
	return int(name.split('_')[0].split('/')[-1].replace('bs', ''))

counter = 0
f = open(filename, 'rt')
for line in f:
	if counter == 0:
		currentFile = line
	elif counter == 1:
		pitch = int(line)
	elif counter == 2:
		value = float(line)
		files.append(currentFile)
		information[currentFile] = list()
		information[currentFile].append(pitch)
		information[currentFile].append(value)
	counter = (counter + 1) % 3

previousImageSet = -1
for i in range(0, len(files)):
	key = files[i]
	currentImageSet = detectImage(key)
	if currentImageSet != previousImageSet:
		previousImageSet = currentImageSet

		if information[key][0] == 0:
			nextKey = files[i+1]
			if information[nextKey][0] == 0:
				correction = information[key][1] + information[nextKey][1]
				correction /= 2
			else:
				correction = information[key][1]
	information[key][1] /= correction
serialize()
