export maxInst=50000000
if [ -z "${useP+x}" ]; then
    echo "useP is not set"
	exit 1
fi
export useP=${useP}
export repo="gem5-hetero-garnet"

export exeP="../../build/build_base_mytest-m64.0000/exchange2_r"

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
export maxInst=$(expr ${maxInst} / ${split})
export core_num=$(expr ${mesh_row} \* ${mesh_row})

cd ${useP}cpu2017/benchspec/CPU/548.exchange2_r/run/run_base_refrate_mytest-m64.0000

${useP}${repo}/build/X86/gem5.opt \
-d ${useP}"gem5-hetero-garnet/scripts/SPEC/m5out/" \
${useP}${repo}/configs/my/se.py \
--cmd=${exeP} \
--option="6" \
--ruby \
--router-latency=3 \
--link-latency=27 \
--num-cpus=${core_num} \
--num-dirs=${core_num} \
--cpu-type=X86O3CPU \
--network=garnet \
--topology=Mesh_XY \
--mesh-rows=${mesh_row} \
--l1d_size=32kB \
--l1i_size=32kB \
--l1d_assoc=8 \
--l1i_assoc=8 \
--num-l2caches=${core_num} \
--l2_size=16MB \
--mem-size=8GB \
--mem-type=DDR4_2400_4x16 \
--routing-algorithm=${routing} \
-I ${maxInst} 
