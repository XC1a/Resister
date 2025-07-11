
mkdir ../resultsRecord/RSA-Cache-Trace-${1}
rm ../resultsRecord/RSA-Cache-Trace-${1}/STATS.txt
for i in {1..1000}
do
# python3 cachePextract.py --traceOri ../cProgram/libgcrypt-1.5.2/exeRepo${1}/RSA-${i} --outfile ../tmp

cd ..
./testForTrace.sh ./cProgram/libgcrypt-1.5.2/exeRepo${1}/RSA-${i} > ./scripts/tmp
cd scripts/
python3 cachePextract.py --traceOri tmp --outfile ../resultsRecord/RSA-Cache-Trace-${1}/key${1}-trace${i} | tee -a ../resultsRecord/RSA-Cache-Trace-${1}/STATS.txt
echo "-->"${i}

done

rm tmp
