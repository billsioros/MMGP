import sys
import os
import time

fileName = sys.argv[1]

while not os.path.isfile(fileName):
    time.sleep(0.001)

fd = open(fileName, "r")

fd.seek(0)

numlines = 1

requests = fd.readline()
perrequest = fd.readline()

while(fd.readline()):
    numlines += 1

requests = str(requests)
perrequest = str(perrequest)
numlines = str(numlines)

requests = requests.replace("\n", "")
requests = requests.replace("\r", "")
perrequest = perrequest.replace("\n", "")
perrequest = perrequest.replace("\r", "")
numlines = numlines.replace("\n", "")
numlines = numlines.replace("\r", "")
 

sys.stdout.write(requests + "\n" + perrequest + "\n" + numlines)
# print str(requests) + "\n" + str(perrequest) + "\n" + str(numlines)