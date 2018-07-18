touch evaluation.txt; rm evaluation.txt; touch evaluation.txt

for ((i = 0; i < 1000; i++))
do
    ./c++/bin/MMGP 30 100 >> evaluation.txt
done

total=0
count=0
for ratio in $(grep "false: *" evaluation.txt | cut -f2 -d ":")
do
    total=$(echo $total + $ratio | bc); ((count++));
done

echo "NN / Multi-Fragment (opted = false)"
echo "scale = 6; $total / $count" | bc

total=0
count=0
for ratio in $(grep "true : *" evaluation.txt | cut -f2 -d ":")
do
    total=$(echo $total + $ratio | bc); ((count++));
done

echo "NN / Multi-Fragment (opted = true)"
echo "scale = 6; $total / $count" | bc