parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

cd "$parent_path"

mkdir -p ./../bin;
g++ -I ../include/ -W -g3 -std=c++14 ./TSP.cpp -o ./../bin/MMGP

./average.sh ./../bin/MMGP "NN: "         1000 results.txt 1
./average.sh ./../bin/MMGP "NN & opt-2: " 1000 results.txt
./average.sh ./../bin/MMGP "MF: "         1000 results.txt
./average.sh ./../bin/MMGP "MF & opt2: "  1000 results.txt
