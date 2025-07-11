import sys
import re

# import openpyxl
# import matplotlib.pyplot as plt


import numpy as np

printNow = False
prefixList = [
    "",
    "xrwang stdout ==>> ",
]
filename = ""
filename1 = ""


sendInterVal = 2
# RecvCycle = {}
LatCycle = {}
SendCycle = {}


def getLineEnc(fileLines):
    findLine = {}
    RecvCycle = {}
    for line in fileLines:
        # print(line)
        for prefix in prefixList:
            lens = len(prefix)
            if line[0:lens] == prefix:
                if re.search(r"CPU R", line) != None:
                    # vaule=re.findall(r"\b\d+\.?\d+\b",line)
                    vaule = re.findall(r"\b\d+\b", line)
                    # print(vaule)
                    assert len(vaule) != 0
                    if int(vaule[0]) not in RecvCycle:
                        RecvCycle[int(vaule[0])] = []
                    RecvCycle[int(vaule[0])].append(int(vaule[1]))
                if re.search(r"CPU S", line) != None:
                    # vaule=re.findall(r"\b\d+\.?\d+\b",line)
                    vaule = re.findall(r"\b\d+\b", line)
                    # print(vaule)
                    assert len(vaule) != 0
                    if int(vaule[0]) not in SendCycle:
                        SendCycle[int(vaule[0])] = []
                    SendCycle[int(vaule[0])].append(int(vaule[1]))

    if printNow:
        print(RecvCycle)
    return RecvCycle, SendCycle


def getGem5LineEnc(fileLines, node):
    LatCycle = []
    for line in fileLines:
        # print(line)
        for prefix in prefixList:
            lens = len(prefix)
            if line[0:lens] == prefix:
                if re.search(r"CPU R", line) != None:
                    # vaule=re.findall(r"\b\d+\.?\d+\b",line)
                    vaule = re.findall(r"\b\d+\b", line)
                    # print(vaule)
                    assert len(vaule) == 3
                    if int(vaule[0]) == node:
                        LatCycle.append(int(vaule[2]))
    return LatCycle

def getGem5LineEncLat(fileLines):
    LatCycle = -1
    for line in fileLines:
        # print(line)
        if re.search(r"Detected", line) != None:
            vaule = re.findall(r"\b\d+\b", line)
            LatCycle = (int(vaule[0]))
    return LatCycle

def output2Excel(dict):
    File = openpyxl.Workbook()
    Sheet = File.create_sheet("sheet0")
    i = 1
    j = 2
    for key in dict:
        Sheet.cell(row=i, column=1, value=(key))
        for v in dict[key]:
            Sheet.cell(row=i, column=j, value=(v))
            j += 1
        i += 1
        j = 2
    File.save("/home/wxrqw/gem5-hetero-garnet/resultsRecord/outExcel001.xls")


def getLat(recv, send):
    i = 0
    LatCycle = []
    for Cycle in recv:
        LatCycle.append(Cycle - send[i])
        i = i + 1
    return LatCycle


def max_normalize(arr):
    min_val = np.min(arr)
    max_val = np.max(arr)
    normalized_arr = (arr) / (max_val)
    return normalized_arr


def lineChart(Lat, LatAfter, o_dis, figName, sampleCycle):
    x = []
    for i in range(0, len(Lat)):
        x.append(i)
    print(Lat)
    print(LatAfter)

    cor = [abs(a - b) for a, b in zip(LatAfter, Lat)]
    print(cor)

    xLimit = 10000 / sampleCycle

    ax01 = plt.subplot(1, 2, 1)
    ax02 = plt.subplot(1, 2, 2)
    if o_dis == False:
        ax01.stem(x, Lat, bottom=150, markerfmt=".")
        ax02.stem(x, LatAfter, bottom=150, markerfmt=".")

        y1max = np.amax(Lat)
        y2max = np.amax(LatAfter)
        ymax = 0
        if y1max > y2max:
            ymax = y1max
        else:
            ymax = y2max
        ymax += 2

        ax01.set_xlim(xmax=xLimit)
        ax02.set_xlim(xmax=xLimit)
        ax01.set_xlim(xmin=0)
        ax02.set_xlim(xmin=0)

        ax01.set_ylim(ymax=ymax)
        ax02.set_ylim(ymax=ymax)

        ax01.set_ylabel("Sample Lat. / cycle")
        ax01.set_xlabel("Sample Index")
        ax02.set_xlabel("Sample Index")
        ax01.grid(linestyle=":", axis="both")
        ax02.grid(linestyle=":", axis="both")
        ax01.set_title("Bit = 0")
        ax02.set_title("Bit = 1")
    else:
        plt.plot(x, cor, color="red", linewidth=1)

    # plt.title("Sample per "+str(sampleCycle)+" cycles")
    if figName != "":
        plt.savefig(figName + ".jpg")
    plt.show()


def getLatRela(recvTime):
    x = []
    lat = []
    i = 0
    for cycle in recvTime:
        if i != 0:
            lat.append(cycle - recvTime[i - 1])
            x.append(i)
        i = i + 1
    return lat


filename = ""
mode = 0
figName = ""
i = 0
relativeLat = False
O_DIS = False
sampleC = 50

traceReccord = ""
for line in sys.argv:
    if line == "--Mesh4x4":
        filename = "../resultsRecord/log001"
        InstMode = True
        mode = 1
    elif line == "--recvLat":
        filename = "./oriData/attRSAkey0"
        filename1 = "./oriData/attRSAkey1"
        mode = 2
    elif line == "-rela":
        relativeLat = True
    elif line == "--sampleC":
        sampleC = int(sys.argv[i + 1])
    elif line == "-odis":
        O_DIS = True
    elif line == "--FigName":
        figName = sys.argv[i + 1]
    elif line == "--BER":
        mode = 3
        filename = "../resultsRecord/logBER001"
    elif line == "--file0":
        filename = sys.argv[i + 1]
    elif line == "--file1":
        filename1 = sys.argv[i + 1]

    elif line == "--getTrace":
        mode = 4
        traceReccord = sys.argv[i + 1]
    elif line == "--getLat":
        mode = 5
        traceReccord = sys.argv[i + 1]
    i = i + 1


if mode == 1:
    file = open(filename, "r")
    lines = file.readlines()
    dicFind = getLineEnc(lines)
    # output2Excel(dicFind)
    print(dicFind[6])
    print(SendCycle[6])
elif mode == 2:
    file = open(filename, "r")
    lines = file.readlines()
    dicFind = getLineEnc(lines)

    file2 = open(filename1, "r")
    lines2 = file2.readlines()
    dicFind2 = getLineEnc(lines2)

    # print (dicFind[1][6])

    if relativeLat == False:
        Lat0 = getLat(dicFind[0][6], dicFind[1][6])
        Lat1 = getLat(dicFind2[0][6], dicFind2[1][6])
        lineChart(Lat0, Lat1, O_DIS, figName, sampleC)
    else:
        Lat0 = getLatRela(dicFind[0][6])
        Lat1 = getLatRela(dicFind2[0][6])
        lineChart(Lat0, Lat1, O_DIS, figName, sampleC)
elif mode == 3:
    file = open(filename, "r")
    lines = file.readlines()
    dicFind = getLineEnc(lines)
    print(getLat(dicFind[10]))
    # print(len(getLat(dicFind[10])))

elif mode == 4:
    file = open(filename, "r")
    lines = file.readlines()
    # dicFind = getLineEnc(lines)
    # Lat0 = getLat(dicFind[0][6], dicFind[1][6])
    sum = []
    Lat0 = getGem5LineEnc(lines, 8)
    Lat1 = getGem5LineEnc(lines, 9)
    Lat2 = getGem5LineEnc(lines, 10)
    Lat3 = getGem5LineEnc(lines, 11)
    if Lat0 != []:
        sum.append(Lat0)
    if Lat1 != []:
        sum.append(Lat1)
    if Lat2 != []:
        sum.append(Lat2)
    if Lat3 != []:
        sum.append(Lat3)
    np.save(traceReccord, sum)

elif mode == 5:
    file = open(filename, "r")
    lines = file.readlines()
    Lat = getGem5LineEncLat(lines)
    print(Lat)
