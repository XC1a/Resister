export routing=6
export routing_has="xx"
export thres=500000000
export thres_has="xx"

for var in "$@"
do
	if [ ${var} == "--routing" ]; then
		export routing_has="has"
	elif [ ${routing_has} == "has" ]; then
		export routing=${var}
		export routing_has="NULL"
	elif [ ${var} == "--thres" ]; then
		export thres_has="has"
	elif [ ${thres_has} == "has" ]; then
		export thres=${var}
		export thres_has="NULL"
	fi
done

mkdir syth_record
# for spec in 0.005 0.006
# for spec in 0.005 0.006
# for spec in 0.005 0.006 0.006 0.007 0.008 0.009 0.010 0.011 0.012 0.013 0.014 0.015
for spec in 0.016 0.017 0.018 0.019 0.020 0.021 0.022 0.023 0.024 0.025
do
    ./testForTraceChiplet.sh xxx 999 xx --testCase 0 --routing ${routing} --thres ${thres}
	cp ./m5out/stats.txt ./syth_record/${1}-${spec}
done