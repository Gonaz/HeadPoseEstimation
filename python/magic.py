import os

for i in range(140, 180):
	val = i/1000
	call = "./HeadPoseEstimation " + str(val)  + " > outPitch; python3 processAll.py"
	os.system(call)
