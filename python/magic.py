import os

for i in range(130, 150):
	val = i/1000
	call = "./HeadPoseEstimation " + str(val)  + " > outPitch; python3 processAll.py"
	os.system(call)
