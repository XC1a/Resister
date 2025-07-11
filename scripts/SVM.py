import numpy as np
import sys
import os
import random

from sklearn import datasets
from sklearn.model_selection import train_test_split
from sklearn import svm
from sklearn.metrics import accuracy_score
from sklearn.linear_model import LinearRegression,LogisticRegression
from sklearn.metrics import mean_squared_error

from scipy.signal import find_peaks
from scipy.stats import ranksums # Wilcoxon
from scipy.stats import mannwhitneyu # man
from scipy.stats import chi2_contingency # chi2 

from scipy import stats


classfir = 'SVM'

prefix = ""
sample = 0
Defend = ""
crossSim = False

i = 0
for line in sys.argv:
    if line == "--prefix":
        prefix = sys.argv[i + 1]
    elif line == "--sample":
        sample = int(sys.argv[i + 1])
    elif line == "--defend":
        Defend = "Defend-"
    elif line == "--cross":
        crossSim = True
    i = i + 1

def filterTH(arr):
    # return 0.1*(max(arr)-min(arr)) + min(arr)
    # return min(arr)
    # return sum(arr)/len(arr)
    return 0.01*sum(arr)/len(arr) + min(arr)

def filterByTh(arr):
    thre = filterTH(arr)
    for i in range(len(arr)):
        if arr[i] < thre:
            arr[i] = min(arr)


# here, the prior knowledge check
def priorChecck(key0Series, key1Series):
    matchKnow = 0
    halfLen = int(len(key0Series[0])/2)
    print("Sample length",halfLen)
    assert(len(key0Series) == len(key1Series))
    for i in range(len(key0Series)):
        PeakS_key0, _ = find_peaks(key0Series[i], filterTH(key0Series[i]) ) 
        PeakS_key1, _ = find_peaks(key1Series[i], filterTH(key1Series[i]) )
        # PeakS1_key0, _ = find_peaks(key0Series[i][0:halfLen], filterTH(key0Series[i]) )
        # PeakS2_key0, _ = find_peaks(key0Series[i][halfLen:], filterTH(key0Series[i]) )
        # PeakS1_key1, _ = find_peaks(key1Series[i][0:halfLen], filterTH(key1Series[i]) )
        # PeakS2_key1, _ = find_peaks(key1Series[i][halfLen:], filterTH(key1Series[i]) )

        PeakS_key0_len = len(PeakS_key0)
        PeakS_key1_len = len(PeakS_key1)
        # PeakS1_key0_len = len(PeakS1_key0)
        # PeakS2_key0_len = len(PeakS2_key0)
        # PeakS1_key1_len = len(PeakS1_key1)
        # PeakS2_key1_len = len(PeakS2_key1)        

        formerPeak = 0
        for peak in PeakS_key0:
            if peak < halfLen:
                formerPeak += 1
        PeakS1_key0_len = formerPeak
        PeakS2_key0_len = PeakS_key0_len - formerPeak

        formerPeak = 0
        for peak in PeakS_key1:
            if peak < halfLen:
                formerPeak += 1
        PeakS1_key1_len = formerPeak
        PeakS2_key1_len = PeakS_key1_len - formerPeak
        ######## here cannot use findpeak again

        right = (PeakS_key1_len>PeakS_key0_len) and (PeakS2_key1_len > PeakS2_key0_len) and (PeakS1_key0_len >0) and (PeakS1_key1_len >0) and (PeakS2_key1_len>0)
        if right:
            matchKnow += 1
            # print(PeakS2_key1_len,PeakS2_key0_len)
            # print(key1Series[i][halfLen:])
        # else:
            # print("Now i ",i)
    
    return matchKnow

# here we use two-stage classfier, the first stage is using the prior knowledge
def priorClassfier(X_train_key0,X_train_key1,X_test,Y_test):
    halfLen = int(len(X_train_key0[0])/2)
    PeakS2_key0_avg = 0
    PeakS_key0_avg = 0
    PeakS2_key1_avg = 0
    PeakS_key1_avg = 0
    for i in range(len(X_train_key0)):
        PeakS_key0, _ = find_peaks(X_train_key0[i], filterTH(X_train_key0[i]))
        # PeakS2_key0, _ = find_peaks(X_train_key0[i][halfLen:], filterTH(X_train_key0[i]))
        formerPeak = 0
        for peak in PeakS_key0:
            if peak < halfLen:
                formerPeak += 1
        PeakS2_key0_len = len(PeakS_key0)- formerPeak

        PeakS_key0_avg += len(PeakS_key0)
        PeakS2_key0_avg += PeakS2_key0_len

    for i in range(len(X_train_key1)):
        PeakS_key1, _ = find_peaks(X_train_key1[i], filterTH(X_train_key1[i]) )
        # PeakS2_key1, _ = find_peaks(X_train_key1[i][halfLen:], filterTH(X_train_key1[i]))
        formerPeak = 0
        for peak in PeakS_key1:
            if peak < halfLen:
                formerPeak += 1
        PeakS2_key1_len = len(PeakS_key1)- formerPeak

        PeakS_key1_avg += len(PeakS_key1)
        PeakS2_key1_avg += PeakS2_key1_len


    boundS = ((PeakS_key0_avg/len(X_train_key0)) + (PeakS_key1_avg/len(X_train_key1))) / 2
    boundS2 = ((PeakS2_key0_avg/len(X_train_key0)) + (PeakS2_key1_avg/len(X_train_key1))) / 2
    print("the margin is ",boundS,boundS2)

    correct_cnt = 0
    not_match = 0
    for i in range(len(X_test)):
        PeakS, _ = find_peaks(X_test[i], filterTH(X_test[i]) )
        # PeakS2, _ = find_peaks(X_test[i][halfLen:], filterTH(X_test[i]))
        formerPeak = 0
        for peak in PeakS:
            if peak < halfLen:
                formerPeak += 1
        PeakS2_len = len(PeakS)- formerPeak

        PeakS_len = len(PeakS)
        PeakS2_len = PeakS2_len

        if(PeakS_len > boundS and PeakS2_len >boundS2):
            res = 1
            if (res == Y_test[i]):
                correct_cnt += 1
        elif(PeakS_len < boundS and PeakS2_len < boundS2):
            res = 0
            if (res == Y_test[i]):
                correct_cnt += 1
        else:
            not_match += 1
            res = random.randrange(0,1)
            if (res == Y_test[i]):
                correct_cnt += 1
    
    return correct_cnt, not_match

            



X = []
y = []

X_defend = []
y_defend = []

### time length
totCycle = 10000
sampleT = int(totCycle / sample)

dir_key0 = prefix + str(sample) + "cycle-key0"
dir_key1 = prefix + str(sample) + "cycle-key1"

items = os.listdir(dir_key0)
for item in items:
    fileP = os.path.join(dir_key0, item)
    X.append(np.load(fileP)[0:sampleT])
    y.append(0)

items = os.listdir(dir_key1)
for item in items:
    fileP = os.path.join(dir_key1, item)
    X.append(np.load(fileP)[0:sampleT])
    y.append(1)

if Defend != "":
    dir_key0 = prefix + Defend + str(sample) + "cycle-key0"
    dir_key1 = prefix + Defend + str(sample) + "cycle-key1"

    items = os.listdir(dir_key0)
    for item in items:
        fileP = os.path.join(dir_key0, item)
        X_defend.append(np.load(fileP)[0:sampleT])
        y_defend.append(0)

    items = os.listdir(dir_key1)
    for item in items:
        fileP = os.path.join(dir_key1, item)
        X_defend.append(np.load(fileP)[0:sampleT])
        y_defend.append(1)

    # print(X[0],X_defend[0])

cross_X = []
cross_Y = []
# print(X[0],X_defend[1])
# print(X[1000],X_defend[1003])
if crossSim:
    for i in range(500):
        cross_X.append(X[i])
        cross_Y.append(0)
        cross_X.append(X_defend[500+i])
        cross_Y.append(0)
    for i in range(500):
        cross_X.append(X[1000+i])
        cross_Y.append(1)
        cross_X.append(X_defend[1500+i])
        cross_Y.append(1)



peakNormal = 0
peakNormal = priorChecck(X[:1000],X[1000:])

print("peak should = 1000 ?: ",peakNormal)

peak0, _ = find_peaks(X[999])
peak1, _ = find_peaks(X[1999])

halfL = int(sampleT/2)
formerPeak = 0
for peak in peak0:
    if peak < halfL:
        formerPeak += 1
print("Sample key0", peak0, max(X[999]))
print("S1 S2 are",formerPeak,len(peak0)-formerPeak)

formerPeak = 0
for peak in peak1:
    if peak < halfL:
        formerPeak += 1
print("Sample key1", peak1, max(X[1999]))
print("S1 S2 are",formerPeak,len(peak1)-formerPeak)
# print(X[0])


X_for_prior_classifier = X[800:1000] + X[1800:]
Y_for_prior_classifier = [0 for i in range(200)]+ [1 for i in range(200)]
correct, not_match = priorClassfier(X[:800],X[1000:1600],X_for_prior_classifier,Y_for_prior_classifier)
print("stage 1 classsfier",correct/400,not_match)

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

X_train_defend = [] 
X_test_defend = []
y_train_defend = []
y_test_defend = [] 
if Defend != "":
    X_train_defend, X_test_defend, y_train_defend, y_test_defend = train_test_split(
        X_defend, y_defend, test_size=0.2, random_state=77
    )

X_train_cross= [] 
X_test_cross= []
y_train_cross= []
y_test_cross= [] 
if cross_Y:
    X_train_cross, X_test_cross, y_train_cross, y_test_cross= train_test_split(
        cross_X, cross_Y, test_size=0.2, random_state=1000
    )

# print(y_train_cross)

#### filter:
# for x in X_train:
#     filterByTh(x)
# for x in X_test:
#     filterByTh(x)


y_pred = []
y_pred_defend = []

y_train_attack = []
y_infer_attack = []

y_corss = []
if classfir == 'SVM':
    clf = svm.SVC(kernel="linear", C=1.0)
    clf.fit(X_train, y_train)
    y_pred = clf.predict(X_test)
    if Defend != "":
        y_infer_attack = clf.predict(X_test_defend)
        # clf1 = svm.SVC(kernel="linear", C=1.0)
        clf.fit(X_train_defend, y_train_defend)
        y_train_attack = clf.predict(X_test)
        y_pred_defend = clf.predict(X_test_defend)

    if crossSim:
        clf.fit(X_train_cross, y_train_cross)
        y_corss = clf.predict(X_test_cross)


elif classfir == 'LR':
    lin_reg = LogisticRegression()
    lin_reg.fit(X_train, y_train)
    y_pred = lin_reg.predict(X_test)


# print(y_test)
# print(y_pred)
print("Accuracy: ", accuracy_score(y_test, y_pred))

if Defend != "":
    print("All Defend: ", accuracy_score(y_test_defend, y_pred_defend))
    print("Train Defend: ", accuracy_score(y_train_attack, y_test))
    print("Infer Defend: ", accuracy_score(y_test_defend, y_infer_attack))

if crossSim:
    print("cross simulation Defend: ", accuracy_score(y_corss, y_test_cross))
# print(y_train)
