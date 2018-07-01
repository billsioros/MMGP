
CC = g++
CCFLAGS = -W -g3 -std=c++11

PATH_BIN = ./../../bin/Routing_Algorithm_Binaries

ROUT_NAM = Routing
ROUT_DEP = $(addprefix $(PATH_BIN)/, student.o vector2.o cluster.o main.o)

SDNT_DEP = student.hpp student.cpp
VEC2_DEP = vector2.hpp vector2.cpp
CLST_DEP = student.hpp heap.hpp vector2.hpp cluster.hpp cluster.cpp
MAIN_DEP = cluster.hpp main.cpp

all:
	@echo "\n>: Compiling executable "$(ROUT_NAM)" :<"
	@echo "***"
	mkdir -p $(PATH_BIN)
	make $(PATH_BIN)/$(ROUT_NAM)
	@echo "***\n"

$(PATH_BIN)/$(ROUT_NAM): $(ROUT_DEP)
	$(CC) $(CCFLAGS) $(ROUT_DEP) -o $(PATH_BIN)/$(ROUT_NAM)

$(PATH_BIN)/student.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) student.cpp -c -o $(PATH_BIN)/student.o

$(PATH_BIN)/vector2.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) vector2.cpp -c -o $(PATH_BIN)/vector2.o

$(PATH_BIN)/cluster.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) cluster.cpp -c -o $(PATH_BIN)/cluster.o

$(PATH_BIN)/main.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) main.cpp -c -o $(PATH_BIN)/main.o
