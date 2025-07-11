import numpy as np
import sys
import os
import random

from sklearn import datasets
from sklearn.model_selection import train_test_split
from sklearn import svm
from sklearn.metrics import accuracy_score
from sklearn.linear_model import LinearRegression, LogisticRegression
from sklearn.metrics import mean_squared_error

from scipy.signal import find_peaks
from scipy import stats

import pickle

classfir = "SVM"

prefix = ""
speccial_prefix = ""
trainCase = 0
savePre = ""
Defend = ""
algo= "RSA"
testCase=6
lineSVC= False #unit is hour

i = 0
for line in sys.argv:
    if line == "--prefix":
        prefix = sys.argv[i + 1]
    if line == "--Sprefix":
        speccial_prefix = sys.argv[i + 1]
    if line == "--defend":
        Defend = "Defend-"
    elif line == "--trainCase":
        trainCase = int(sys.argv[i + 1])
    elif line == "--savePre":
        savePre = sys.argv[i + 1]
    elif line == "--algo":
        algo = sys.argv[i + 1]
    elif line == "--testCase":
        testCase = int(sys.argv[i + 1])
    elif line == "-lineSVC":
        lineSVC = True
    i = i + 1

print("Time Limit is ",timeLimit)

onlyinfer = False
if trainCase != 0:
    onlyinfer = True

X = []
y = []

X_defend = []
y_defend = []


sample_list = [40,50,60,70, 80, 90, 100, 110, 120]

onlyinfer = False
if trainCase != 0:
    onlyinfer = True

X = []
y = []

X_defend = []
y_defend = []


def readData(sample: int, prefixAll, totCycle=10000):
    # we first clear all data
    X.clear()
    y.clear()
    X_defend.clear()
    y_defend.clear()

    ### time length
    # totCycle = 10000
    sampleT = int(totCycle / sample)

    dir_key0 = prefixAll + str(sample) + "cycle-key0"
    dir_key1 = prefixAll + str(sample) + "cycle-key1"

    items = os.listdir(dir_key0)
    for item in items:
        fileP = os.path.join(dir_key0, item)
        dictD = np.load(fileP)
        if (dictD.ndim > 1):
            tmp7=[]
            jump = False
            for i in range(len(dictD)):
                if testCase == 7:
                    if len(dictD[i]) != 500 or len(dictD) != 4:
                        jump=True
                    tmp7.extend(dictD[i])
                else:
                    X.append(dictD[i][0:sampleT])
                    y.append(0)
            if jump == True:
                continue
            if testCase == 7:
                y.append(0)
                X.append(tmp7)
        elif testCase == 7:
            continue
        else:
            X.append(dictD[0:sampleT])
            y.append(0)

    items = os.listdir(dir_key1)
    for item in items:
        fileP = os.path.join(dir_key1, item)
        dictD = np.load(fileP)
        if (dictD.ndim > 1):
            tmp7=[]
            jump = False
            for i in range(len(dictD)):
                if testCase == 7:
                    if len(dictD[i]) != 500 or len(dictD) != 4:
                        jump = True
                    tmp7.extend(dictD[i])
                else:
                    X.append(dictD[i][0:sampleT])
                    y.append(1)
            if jump == True:
                continue
            if testCase == 7:
                y.append(1)
                X.append(tmp7)
        elif testCase == 7:
            continue
        else:
            X.append(dictD[0:sampleT])
            y.append(1)


def train_and_save(onlyinfer, savePath=""):

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42
    )
    if onlyinfer == False:
        X_train_defend = []
        X_test_defend = []
        y_train_defend = []
        y_test_defend = []
        y_pred = []
        assert savePath != ""
        if lineSVC == False:
            clf = svm.SVC(kernel="linear", C=1.0)
        else:
            clf = svm.LinearSVC(C=1.0, max_iter=8000, dual=False)
        if timeLimit != -1:
            clf.fit(X_train, y_train, timeLimit*3600)
        else:
            clf.fit(X_train, y_train)
        with open("./SVM_Save/" + savePath, "wb") as f:
            pickle.dump(clf, f)
            y_pred = clf.predict(X_test)
    else:
        with open("./SVM_Save/" + savePath, "rb") as f:
            clf_load = pickle.load(f)
            y_pred = clf_load.predict(X_test)
    return accuracy_score(y_test, y_pred)


if speccial_prefix != '':
    if speccial_prefix[:len('../resultsRecord/log-TimeLocate')] == '../resultsRecord/log-TimeLocate':
        caseString = speccial_prefix+ algo + "-sample-testCase-5-0-0-" + Defend
        loadFileName = (
            savePre + algo + "-sample-testCase-5-0-0-" + Defend + '50'
        )
        readData(50, caseString)
        acc = train_and_save(onlyinfer, loadFileName)
        print(acc)
    if speccial_prefix[:len('../resultsRecord/log-')] == '../resultsRecord/log-':
        caseString = speccial_prefix+ algo + "-sample-testCase-"+str(testCase)+"-0-0-" + Defend
        loadFileName = (
            savePre + algo + "-sample-testCase-"+str(testCase)+"-0-0-" + Defend + '50'
        )
        if algo == 'ECC':
            readData(50, caseString, 17000*50)
        else:
            readData(50, caseString)
        acc = train_and_save(onlyinfer, loadFileName)
        print(acc)

else:
    for sample in sample_list:
        caseString = prefix + algo + "-sample-testCase-1-0-0-" + Defend
        loadFileName = (
            savePre + algo + "-sample-testCase-1-0-0-" + Defend + str(sample)
        )
        readData(sample, caseString)
        acc = train_and_save(onlyinfer, loadFileName)
        print(acc)

    attackV = [50, 50, 100]
    count = 0
    for sample in attackV:
        if count == 0:
            caseString = prefix + algo + "-sample-testCase-1-49-0-" + Defend
            loadFileName = (
                savePre + algo + "-sample-testCase-1-49-0-" + Defend + str(sample)
            )
            readData(sample, caseString)
            acc = train_and_save(onlyinfer, loadFileName)
            print(acc)
        elif count == 1:
            caseString = prefix + algo + "-sample-testCase-3-0-0-" + Defend
            loadFileName = (
                savePre + algo + "-sample-testCase-3-0-0-" + Defend + str(sample)
            )
            readData(sample, caseString)
            acc = train_and_save(onlyinfer, loadFileName)
            print(acc)
        elif count == 2:
            caseString = prefix + algo + "-sample-testCase-4-0-0-" + Defend
            loadFileName = (
                savePre + algo + "-sample-testCase-4-0-0-" + Defend + str(sample)
            )
            readData(sample, caseString)
            acc = train_and_save(onlyinfer, loadFileName)
            print(acc)
        count += 1
