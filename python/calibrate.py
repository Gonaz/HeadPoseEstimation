from pprint import pprint

f = open('positionsPitchNew', 'rt')
pitchDict = {'U':2, 'SU':1, 'SD':-1, 'D':2}
counter = 0
pitch = 0
previousImageSet = 0
currentImageSet = 0
corrections = list()
information = dict()
files = list()

def mean(_list):
	return sum(_list)/len(_list)	

def serialize():
	g = open('positionsPitchNewCal', 'wt')
	for key in files:
		g.write(key)
		g.write(str(information[key][0]) + "\n")
		g.write(str(information[key][1]) + "\n")
	g.close()
	f
def detectPitch(name):
	if name.count('PR') > 0:
		s = name.split('_')
		return pitchDict[s[-2]]
	else:
		return 0

def detectImage(name):
	return int(name.split('_')[0].split('/')[-1].replace('bs', ''))

for line in f:
	if counter == 0:
		currentImageSet = detectImage(line)
		if currentImageSet != previousImageSet:
			#serialize()
			corrections = list()
			previousImageSet = currentImageSet
		currentFile = line
		files.append(currentFile)
		information[currentFile] = list()
	elif counter == 1:
		currentPitch = int(line)
		information[currentFile].append(currentPitch)
	elif counter == 2:
		currentValue = float(line)
		if currentPitch == 0:
			information[currentFile].append(1)
			corrections.append(float(line))
		else:
			correction = mean(corrections)
			if correction == 0:
				information[currentFile].append(float(line))
			else:
				information[currentFile].append(float(line)/correction)
	counter = (counter + 1)%3	

serialize()
f.close()
