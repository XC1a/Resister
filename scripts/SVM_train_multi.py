import os

pList = []
pList.append("python3 SVM_Sequence.py --Sprefix ../resultsRecord/log-COO-Ori- --savePre --testCase 7 COO-Ori-")
pList.append("python3 SVM_Sequence.py --Sprefix ../resultsRecord/log-COO-valiant- --testCase 7 --savePre COO-valiant-")
pList.append("python3 SVM_Sequence.py --Sprefix ../resultsRecord/log-COO-TDMA- --testCase 7 --savePre COO-TDMA-")

percent = [1.0, 0.75, 0.5, 0.4, 0.3, 0.25, 0.2, 0.15]
for p in percent:
    pList.append("python3 SVM_Sequence.py --Sprefix ../resultsRecord/log-COO-"+str(p*100)+"Detour- --testCase 7 --savePre "+ str(p*100) +"COO-")

i = 0
for line in pList:
    print(i,line)
    os.system(line)
    i += 1