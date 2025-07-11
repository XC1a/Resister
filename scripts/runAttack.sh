
export recordOri=""
export protect=""
export exeP=""
export whiteND=0
export whiteND_LOW=0
export whiteNDhas="xx"

export testCase=0
export testCase_has="xx"

export SpecPrefix=""
export SpecPrefix_has="xx"

export routing=6
export routing_has="xx"

export getLat="None"

for var in "$@"
do
	if [ ${var} == "--SaveOri" ]; then
		export recordOri="--SaveOri"
	elif [ ${var} == "--protect" ]; then
		export protect="--protect"
	elif [ ${var} == "-lat" ]; then
		export getLat="-lat"
	elif [ ${var} == "--testCase" ]; then
		export testCase_has="has"
	elif [ ${testCase_has} == "has" ]; then
		export testCase=${var}
		export testCase_has="NULL"
	elif [ ${var} == "-s" ]; then
		export exeP="-s3"
	elif [ ${whiteNDhas} == "has" ]; then
		export whiteND=${var}
		export whiteNDhas="has1"
	elif [ ${var} == "-WD" ]; then
		export whiteNDhas="has"
	elif [ ${whiteNDhas} == "has1" ]; then
		export whiteND_LOW=${var}
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
	fi
done
echo ${whiteND} ${whiteND_LOW}

# the basic attack -> ATT
for spec in 40 50 60 70 80 90 100 110
do
    ./getChipletTrace.sh 0 ${spec} ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 1 --routing ${routing} --prefix ${SpecPrefix} ${getLat}
    ./getChipletTrace.sh 1 ${spec} ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 1 --routing ${routing} --prefix ${SpecPrefix} ${getLat}
done

# ATT-V1
./getChipletTrace.sh 0 50 ${recordOri} ${protect} ${exeP} -WD 49 0 --testCase 1 --routing ${routing} --prefix ${SpecPrefix} ${getLat}
./getChipletTrace.sh 1 50 ${recordOri} ${protect} ${exeP} -WD 49 0 --testCase 1 --routing ${routing} --prefix ${SpecPrefix} ${getLat}

# ATT-V2
./getChipletTrace.sh 0 50 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 3 --routing ${routing} --prefix ${SpecPrefix} ${getLat}
./getChipletTrace.sh 1 50 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 3 --routing ${routing} --prefix ${SpecPrefix} ${getLat}

# ATT-V3
./getChipletTrace.sh 0 100 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 4 --routing ${routing} --prefix ${SpecPrefix} ${getLat}
./getChipletTrace.sh 1 100 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 4 --routing ${routing} --prefix ${SpecPrefix} ${getLat}

# vector time changes, Dynamic-Attack in the paper
./getChipletTrace.sh 0 50 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 5 --routing ${routing} --prefix ${SpecPrefix} ${getLat}
./getChipletTrace.sh 1 50 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 5 --routing ${routing} --prefix ${SpecPrefix} ${getLat}

# Colluding Attack in the paper
./getChipletTrace.sh 0 50 ${recordOri} ${protect} ${exeP} -WD ${whiteND} ${whiteND_LOW} --testCase 7 --routing ${routing} --prefix ${SpecPrefix} ${getLat}
./getChipletTrace.sh 1 50 ${recordOri} ${protect} ${exeP} -WD ${whiteND} ${whiteND_LOW} --testCase 7 --routing ${routing} --prefix ${SpecPrefix} ${getLat}

# #ECC
./getChipletTrace.sh 0 50 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 6 --algo ECC --routing ${routing} --prefix ${SpecPrefix}
./getChipletTrace.sh 1 50 ${recordOri} ${protect} ${exeP} -WD 0 0 --testCase 6 --algo ECC --routing ${routing} --prefix ${SpecPrefix}
