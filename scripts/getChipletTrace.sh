#!/bin/bash


export recordOri="None"
export protect=""
export exeP=""
export whiteND=0
export whiteND_LOW=0
export whiteNDhas="xx"
export whiteFilePrefix=""
export whiteFilePrefix1=""
export testCase=0
export testCase_has="xx"
export routing=6
export routing_has="xx"

export SpecPrefix=""
export SpecPrefix_has="xx"
export algo="RSA"
export algo_has="xx"

export getLat="None"

for var in "$@"
do
	if [ ${var} == "--SaveOri" ]; then
		export recordOri="r"
	elif [ ${var} == "--protect" ]; then
		export protect="Defend-"
	elif [ ${var} == "-s3" ]; then
		export exeP="-s3"
	elif [ ${var} == "-lat" ]; then
		export getLat="Has"
	elif [ ${var} == "--testCase" ]; then
		export testCase_has="has"
	elif [ ${testCase_has} == "has" ]; then
		export testCase=${var}
		export whiteFilePrefix1="testCase-"${var}"-"
		# export whiteFilePrefix=""
		export testCase_has="NULL"
	elif [ ${var} == "-WD" ]; then
		export whiteNDhas="has"
	elif [ ${whiteNDhas} == "has" ]; then
		export whiteND=${var}
		export whiteNDhas="has1"
	elif [ ${whiteNDhas} == "has1" ]; then
		export whiteND_LOW=${var}
		export whiteFilePrefix=${whiteND}"-"${var}"-"
		# export whiteFilePrefix1=${var}"-"
		echo ${whiteFilePrefix1}${whiteFilePrefix}
		export whiteNDhas="NULL"
	elif [ ${var} == "--prefix" ]; then
		export SpecPrefix_has="has"
	elif [ ${SpecPrefix_has} == "has" ]; then
		export SpecPrefix=${var}
		export SpecPrefix_has="NULL"
	elif [ ${var} == "--routing" ]; then
		export routing_has="has"
	elif [ ${routing_has} == "has" ]; then
		export routing=${var}
		export routing_has="NULL"
	elif [ ${var} == "--algo" ]; then
		export algo_has="has"
	elif [ ${algo_has} == "has" ]; then
		export algo=${var}
		export algo_has="NULL"
	fi
done


# make the dir to record the res
if [[ ${recordOri} == "r" ]]; then
mkdir ../resultsRecord/${SpecPrefix}${algo}-sample-${whiteFilePrefix1}${whiteFilePrefix}${protect}${2}cycle-key${1}-Ori
fi
mkdir ../resultsRecord/${SpecPrefix}${algo}-sample-${whiteFilePrefix1}${whiteFilePrefix}${protect}${2}cycle-key${1}

for i in {1..1000}
do
cd ..
./testForTraceChiplet.sh resultsRecord/${algo}-Cache-Trace-${1}/key${1}-trace${i} ${2} ${exeP} -WD ${whiteND} ${whiteND_LOW} --testCase ${testCase} --routing ${routing} > scripts/tmp

if [[ ${recordOri} == "r" ]]; then
cp scripts/tmp resultsRecord/${SpecPrefix}${algo}-sample-${whiteFilePrefix1}${whiteFilePrefix}${protect}${2}cycle-key${1}-Ori/ori-${i}
fi

cd scripts/
if [[ ${getLat} == "None" ]]; then
python3 getDest.py --getTrace ../resultsRecord/${SpecPrefix}${algo}-sample-${whiteFilePrefix1}${whiteFilePrefix}${protect}${2}cycle-key${1}/res-${i} --file0 tmp
else
python3 getDest.py --getLat ../resultsRecord/${SpecPrefix}${algo}-sample-${whiteFilePrefix1}${whiteFilePrefix}${protect}${2}cycle-key${1}/res-${i} --file0 tmp | tee -a detection_lat
fi


done

if [[ ${getLat} == "Has" ]]; then
mkdir ../resultsRecord/${SpecPrefix}${algo}-${whiteFilePrefix1}${whiteFilePrefix}${protect}${2}cycle-key${1}/
cp detection_lat ../resultsRecord/${SpecPrefix}${algo}-${whiteFilePrefix1}${whiteFilePrefix}${protect}${2}cycle-key${1}/
else

rm tmp
rm detection_lat
