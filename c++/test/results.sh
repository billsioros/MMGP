parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

cd "$parent_path"

g++ -I ../include/ -W -g3 -std=c++14 ./TSP.cpp -o ./TSP

./average.sh ./TSP "NN: "         1000 results.txt 1
./average.sh ./TSP "NN & opt-2: " 1000 results.txt
./average.sh ./TSP "MF: "         1000 results.txt
./average.sh ./TSP "MF & opt2: "  1000 results.txt
./average.sh ./TSP "opt2: "       1000 results.txt
