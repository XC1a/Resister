i_before="old_text"
i_now="new_text"

for i in {1..1000}
do
i_now=$i
sed -i "43s/$i_before/$i_now/g" ./mpi/mpi-pow.c
i_before=$i

make
cd ./tests
./compileTest.sh
cp RSA ../exeRepo1/RSA-$i
cd ..
done
