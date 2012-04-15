def emptyDic():
	return {-2:0, -1:0, 0:0, 1:0, 2:0}
def getMax(dic):
	currentMax = 0
	for k,v in dic.items():
		if v > currentMax:
			currentMax = v
	maxList = list()
	for k,v in dic.items():
		if v == currentMax:
			maxList.append(k)
	return maxList
def div(dic1, dic2):
	result = emptyDic()
	for k,v in dic1.items():
		result[k] = dic1[k]/dic2[k]
	return result
def sum(dic):
	result = 0
	for k,v in dic.items():
		result += v
	return result

correct = emptyDic()
totals = emptyDic()
dic = {"D":-2, "SD":-1, "SU":1, "U":2}
tmp = emptyDic()
totalTies = 0
correctTies = 0

f = open('outPitch')
print(float(f.readline()), end="\t")
f.readline()
counter = 0
for line in f:
	line = line.strip()
	if line.startswith("Test"):
		split = line.split("_")
		if split[1] == "N":
			expected = 0
		else:
			expected = dic[split[2]]
		totals[expected] += 1 
	elif line.startswith("/home"):
		break
	elif not len(line) == 0:
		counter += 1
		split = line.split(" -> ")
		tmp[int(split[0])] += int(split[1])
	if counter == 5:
		maxList = getMax(tmp)
		if len(maxList) == 1:
			if maxList[0] == expected:
				correct[expected] += 1
		else:
			#print(expected,"->",maxList)
			totalTies += 1
			if expected in maxList:
				correctTies += 1
		tmp = emptyDic()
		counter = 0
#print("Correct distribution:", correct)
#print("Correct normalized distribution:", div(correct, totals))
#print("Sum total", sum(totals))
#print("Sum correct", sum(correct))
#print(sum(correct)/sum(totals))
print("Ties", totalTies, correctTies)
