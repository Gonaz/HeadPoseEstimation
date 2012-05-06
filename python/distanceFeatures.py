f = open('../training/positionsYawBetter', 'rt')
g = open('../training/positionsYawOrig', 'rt')

counter = 0
distance1 = 0
distance2 = 0

for linef, lineg in zip(f,g):
	if counter == 0:
		counter += 1
	elif counter == 1:
		diff = abs(int(linef) - int(lineg))
		distance1 += diff		
		counter += 1
	elif counter == 2:
		diff = abs(int(linef) - int(lineg))
		distance2 += diff		
		counter = 0

print("Distance 1:", (distance1/244))
print("Distance 2:", (distance2/244))
