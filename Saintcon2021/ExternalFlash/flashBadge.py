#! /usr/bin/env python3

import serial, sys, os
from time import sleep

def usage():
	print("usage: %s serial_port flash_image"%sys.argv[0])
	sys.exit(-1)



if __name__ == '__main__':
	if len(sys.argv) < 3:
		usage()

	total = os.stat(sys.argv[2]).st_size
	
	with serial.Serial(sys.argv[1], timeout=10) as ser:
		ser.write(b'`')
		line = ser.readline()
		print(line)
		if b'rewrite' not in line:
			print("Unexpected response, quitting")
			sys.exit(-2)
		ser.write(b'Y')
		line = ser.readline()
		print(line)
		if b'Erasing' not in line:
			print("Unexpected response, quitting")
			sys.exit(-2)
		while (True):
			line = ser.readline()
			if b'Ready' in line:
				break
		print(line)
		with open(sys.argv[2], 'rb') as f:
			chunk = f.read(256)
			while (chunk):
				ser.write(chunk)
				#ser.readline()
				print("%4.1fKB %2d%%"%(f.tell()/1024.0, 100*f.tell()/total), end='\r')
				sleep(0.001) #allow chip a moment to keep up
				chunk = f.read(256)
	print("%4.1fKB 100%% Done!"%(total/1024.0))
