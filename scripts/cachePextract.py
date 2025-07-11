import sys
import re


identifyToken = "---- New Bit ----"

def outStdTrace(lines, outputName):
    statePatterOn = False
    outputFormart = []
    firstIn = 0
    InCycle = 0
    # record from the first to the last one
    totCnt = 0
    ptrCnt = 0
    for line in lines:
        if line[:len(identifyToken)] == identifyToken:
            totCnt  += 1

    for line in lines:
        if line[:len(identifyToken)] == identifyToken:
            ptrCnt += 1
        if line[:len(identifyToken)] == identifyToken and ptrCnt == 1:
            statePatterOn = True
        elif line[:len(identifyToken)] == identifyToken and ptrCnt == totCnt:
            statePatterOn = False

        if statePatterOn==True and (re.search(r"LtwoCache--", line) != None):
            cycle = re.findall(r"\b\d+\b", line)
            RW = line[-2]
            if firstIn == 0:
                InCycle = int(cycle[0])
            assert len(cycle) != 0
            outputFormart.append([RW,int(cycle[0])-InCycle])
            firstIn += 1
    print("Out---- ---- Here are ",firstIn, " W/R found")
    # print(outputFormart)
    with open(outputName, 'w') as fw:
        for entry in outputFormart:
            fw.write(str(entry[0])) 
            fw.write(" ") 
            fw.write(str(entry[1])) 
            fw.write("\n") 

        


fileIn=""
fileOut=""
i = 0
for line in sys.argv:
    if line == "--traceOri":
        fileIn = sys.argv[i+1]
    elif line == "--outfile":
        fileOut = sys.argv[i+1]
    i = i + 1

file = open(fileIn, "r")
lines = file.readlines()
outStdTrace(lines, fileOut)