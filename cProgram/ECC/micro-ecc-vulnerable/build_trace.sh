i_before="old_text"
i_now="new_text"

for i in {1..1000}
do
i_now=$i
sed -i "7s/$i_before/$i_now/g" ./uECC.c
i_before=$i

./buildtest.sh 
cp testprogram ./exeRepo${1}/ECC-$i
done


