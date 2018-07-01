
CC = g++
CCFLAGS = -W -g3 -std=c++11

PATH_SRC = ./src/
PATH_BIN = ./bin/

ROUT_DEP = $(addprefix $(PATH_BIN), student.o vector2.o cluster.o main.o)

SDNT_DEP = $(addprefix $(PATH_SRC), student.hpp student.cpp)
VEC2_DEP = $(addprefix $(PATH_SRC), vector2.hpp vector2.cpp)
CLST_DEP = $(addprefix $(PATH_SRC), student.hpp heap.hpp vector2.hpp cluster.hpp cluster.cpp)
MAIN_DEP = $(addprefix $(PATH_SRC), cluster.hpp main.cpp)

all:
	@echo -e ">: Compiling executable "routing"\n"
	mkdir -p $(PATH_BIN)
	make $(PATH_BIN)routing

$(PATH_BIN)routing: $(ROUT_DEP)
	$(CC) $(CCFLAGS) $(ROUT_DEP) -o $(PATH_BIN)routing

$(PATH_BIN)student.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_SRC)student.cpp -c -o $(PATH_BIN)student.o

$(PATH_BIN)vector2.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_SRC)vector2.cpp -c -o $(PATH_BIN)vector2.o

$(PATH_BIN)cluster.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_SRC)cluster.cpp -c -o $(PATH_BIN)cluster.o

$(PATH_BIN)main.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_SRC)main.cpp -c -o $(PATH_BIN)main.o

.PHONY: clean
clean:
	rm -rI $(PATH_BIN)
