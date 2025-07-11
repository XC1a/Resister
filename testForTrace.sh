# LD_LIBRARY_PATH="/home/wxrqw/gem5-hetero-garnet/cProgram/libgcrypt-1.5.2/src/.libs:$LD_LIBRARY_PATH"
# export LD_LIBRARY_PATH

# ./build/X86/gem5.opt configs/example/se.py -c ./cProgram/libgcrypt-1.5.2/tests/RSA -o rsa_T \
./build/X86/gem5.opt configs/example/se.py -c ${1} -o rsa_T \
--caches \
--l2cache \
--cpu-type=X86O3CPU \
--num-cpus=1 \
--l1d_size=32kB  \
--l1i_size=32kB \
--l1d_assoc=8 \
--l1i_assoc=8 \
--l2_size=16MB \
--l2_assoc=8 \
--mem-size=4GB \
--mem-type=DDR4_2400_4x16
