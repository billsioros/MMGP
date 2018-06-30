
CC = g++
CCFLAGS = -W -g3 -std=c++11

PATH_SRC = ./src/
PATH_BIN = ./bin/

VEC2_DEP = $(addprefix $(PATH_SRC), vector2.hpp vector2.cpp)
CLST_DEP = $(addprefix $(PATH_SRC), heap.hpp vector2.hpp cluster.hpp cluster.cpp)
MAIN_DEP = $(addprefix $(PATH_SRC), cluster.hpp main.cpp)

ROUT_DEP = $(addprefix $(PATH_BIN), vector2.o cluster.o main.o)

all:
	@echo -e ">: Compiling executable "route"\n"
	mkdir -p $(PATH_BIN)
	make $(PATH_BIN)route

$(PATH_BIN)route: $(ROUT_DEP)
	$(CC) $(CCFLAGS) $(ROUT_DEP) -o $(PATH_BIN)route

$(PATH_BIN)vector2.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_SRC)vector2.cpp -c -o $(PATH_BIN)vector2.o

$(PATH_BIN)cluster.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_SRC)cluster.cpp -c -o $(PATH_BIN)cluster.o

$(PATH_BIN)main.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_SRC)main.cpp -c -o $(PATH_BIN)main.o

.PHONY: clean
clean:
	rm -rI $(PATH_BIN)
