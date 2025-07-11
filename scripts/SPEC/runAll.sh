export routing=6
export routing_has="xx"
export mesh_row=4
export mesh_has="xx"
export split=1
export split_has="xx"

for var in "$@"
do
	if [ ${var} == "--routing" ]; then
		export routing_has="has"
	elif [ ${routing_has} == "has" ]; then
		export routing=${var}
		export routing_has="NULL"
	elif [ ${var} == "--mesh" ]; then
		export mesh_has="has"
	elif [ ${mesh_has} == "has" ]; then
		export mesh_row=${var}
		export mesh_has="NULL"
	elif [ ${var} == "--split" ]; then
		export split_has="has"
	elif [ ${split_has} == "has" ]; then
		export split=${var}
		export split_has="NULL"
	fi
done


for spec in 500 505 507 508 510 511 525 526 531 538 541 544 548 549 
do
    ./run_${spec}.sh --routing ${routing} --mesh ${mesh_row} --split ${split}
    cp m5out/stats.txt record/${spec}-${1}
done