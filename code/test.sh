!bin/bash
array=(5 5 5 5 5 5 5 5 5)
# array=(2 3 3 4 4 4 5 5 5) #for 2-64
# array=(2 3 3 4 4 4 4 4 4) #for point
# array=(32 32 32 32 32 32 32 32 32) #for point
# array=(4 4 4 4 4 5 6 5 5) #for osm
# array=(10 11 12 13 14 14 14 14 14) #for books
# for ((i=14,j=2; i<=26,j<9; i+=2,j++))
# do
# echo './main' 50000000 10000000 $i 0 ${array[j]}
# ./main 50000000 10000000 $i 0 ${array[j]}
# done
for ((i=14; i<=26; i+=2))
do
echo ./main 50000000 10000000 $i 1 1
./main 50000000 10000000 $i 1 1
done
# for ((i=26; i<=26; i+=2))
# do
# echo 'cpulimit --limit=100 ./main 5' + $i
# cpulimit --limit=100 ./main $i 5
# echo 'cpulimit --limit=80 ./main 5' + $i
# cpulimit --limit=80 ./main $i 5
# echo 'cpulimit --limit=60 ./main 5' + $i
# cpulimit --limit=60 ./main $i 5
# echo 'cpulimit --limit=40 ./main 5' + $i
# cpulimit --limit=40 ./main $i 5
# echo 'cpulimit --limit=20 ./main 5' + $i
# cpulimit --limit=20 ./main $i 5
# done