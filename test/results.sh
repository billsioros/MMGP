#!/bin/bash

./average.sh "NN: "        YES results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Morning
./average.sh "OPT2: "      NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Morning
./average.sh "SA: "        NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Morning
./average.sh "OPT2-SA: "   NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Morning

./average.sh "NN: "        YES results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Noon
./average.sh "OPT2: "      NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Noon
./average.sh "SA: "        NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Noon
./average.sh "OPT2-SA: "   NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Noon

./average.sh "NN: "        YES results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Study
./average.sh "OPT2: "      NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Study
./average.sh "SA: "        NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Study
./average.sh "OPT2-SA: "   NO  results.txt ../bin/MMGP -db /home/massiva/Documents/Projects/MMGP/data/MMGP_Data.db -dp Study
