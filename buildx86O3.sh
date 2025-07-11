# /usr/bin/env python3 $(which scons) build/NULL/gem5.opt PROTOCOL=Garnet_standalone

#!/bin/sh

#scons build/NULL/gem5.debug -j 112

#!/bin/bash

export policy="resister"
export policy_has="xxx"

export percent=1.0
export percent_has="xxx"


export coreNum=16
export coreNum_has="xxx"

for var in "$@"
do
	if [ ${var} == "--policy" ]; then
		export policy_has="has"
	elif [ ${policy_has} == "has" ]; then
		export policy=${var}
		export policy_has="NULL"
	elif [ ${var} == "--percent" ]; then
		export percent_has="has"
	elif [ ${percent_has} == "has" ]; then
		export percent=${var}
		export percent_has="NULL"
	elif [ ${var} == "--core" ]; then
		export coreNum_has="has"
	elif [ ${coreNum_has} == "has" ]; then
		export coreNum=${var}
		export coreNum_has="NULL"
	fi
done


if [ ${policy} == "resister" ]; then

sed -i '57s/.*/#define InputStruct 2/' src/mem/ruby/network/garnet/InputUnit.hh

elif [ ${policy} == "ori" ]; then

sed -i '57s/.*/#define InputStruct 0/' src/mem/ruby/network/garnet/InputUnit.hh

elif [ ${policy} == "valiant" ]; then

sed -i '57s/.*/#define InputStruct 3/' src/mem/ruby/network/garnet/InputUnit.hh

elif [ ${policy} == "TDMA" ]; then

sed -i '57s/.*/#define InputStruct 4/' src/mem/ruby/network/garnet/InputUnit.hh

sed -i "64s/.*/#define CORE_NUM ${coreNum}/g" src/mem/ruby/network/garnet/InputUnit.hh

fi


sed -i '62s/\([0-9]*\.[0-9]*\)/'"$percent"'/g' src/mem/ruby/network/garnet/InputUnit.hh





scons build/X86/gem5.opt PROTOCOL=MESI_Two_Level -j8

