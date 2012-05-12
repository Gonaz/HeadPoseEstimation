import os

best = 0
bestVal = list()
def processOutput():
	global best, bestVal
	f = open("outPitch", "rt")
	f.readline()
	s = f.readline().split("/")
	if int(s[0]) > best:
		best = int(s[0])
		bestVal = [val]
	elif int(s[0]) == best:
		bestVal.append(val)

def update():
	distance = end - start
	p = int((i-start)/distance*100)
	if p%2 == 0:
		print(str(p)+"%","\tbest accuracy", best, "\tWith", bestVal)

start = 1
end = 1000
for i in range(start, end):
	val = i/1000
	call = "./HeadPoseEstimation " + str(val) + "> outPitch"
	os.system(call)
	processOutput()
	update()

print("Best accuracy", best, "\tWith", bestVal)
