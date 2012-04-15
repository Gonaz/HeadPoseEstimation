import os

for i in range(1380, 1420):
	val = i/10000
	call = "./HeadPoseEstimation " + str(val)  + " > outPitch; python3 processAll.py"
	os.system(call)
