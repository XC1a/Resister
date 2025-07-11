# here are some stardards refer to related works (from recent DAC paper)
# router latency = 2, and link latency = 27

export exeplace="pc"
export prefix=""
export whiteND=0
export whiteND_LOW=0
export whiteNDhas="xx"
export testCase=0
export testCase_has="xx"
export routing=6
export routing_has="xx"
export inj=0.005
export inj_has="xx"
export thres=500000000
export thres_has="xx"

for var in "$@"
do
	if [ ${var} == "-pc" ]; then
		export exeplace="pc"
	elif [ ${var} == "-s3" ]; then
		export exeplace="s3"
	elif [ ${var} == "--testCase" ]; then
		export testCase_has="has"
	elif [ ${testCase_has} == "has" ]; then
		export testCase=${var}
		export testCase_has="NULL"
	elif [ ${whiteNDhas} == "has" ]; then
		export whiteND=${var}
		export whiteNDhas="has1"
	elif [ ${var} == "-WD" ]; then
		export whiteNDhas="has"
	elif [ ${whiteNDhas} == "has1" ]; then
		export whiteND_LOW=${var}
		export whiteNDhas="NULL"
	elif [ ${var} == "--routing" ]; then
		export routing_has="has"
	elif [ ${routing_has} == "has" ]; then
		export routing=${var}
		export routing_has="NULL"
	elif [ ${var} == "--inj" ]; then
		export inj_has="has"
	elif [ ${inj_has} == "has" ]; then
		export inj=${var}
		export inj_has="NULL"
	elif [ ${var} == "--thres" ]; then
		export thres_has="has"
	elif [ ${thres_has} == "has" ]; then
		export thres=${var}
		export thres_has="NULL"
	fi
done

if [ ${exeplace} == "pc" ]; then
prefix="/home/wxrqw/gem5-hetero-garnet/"
else
prefix="/home/wangxinrui/gem5-hetero-garnet/"
fi


./build/NULL/gem5.opt configs/my/testForMeshTrace.py \
--sys-clock=2GHz \
--ruby-clock=2GHz \
--num-cpus=16 \
--num-dirs=16 \
--network=garnet \
--topology=Mesh_XY \
--mesh-rows=4 \
--routing-algorithm=${routing} \
--sim-cycles=8000000000 \
--synthetic=uniform_random \
--injectionrate=${inj} \
--router-latency=3 \
--link-latency=27 \
--TraceFile=${prefix}${1} \
--sampleT=${2} \
--testCase=${testCase} \
--whiteND=${whiteND} \
--whiteND_LOW=${whiteND_LOW} \
--garnet-deadlock-threshold=${thres}
# --TraceFile=${prefix}"scripts/CachePattern/RSAkey0"
