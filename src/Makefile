
CCNAME = Routing
CC = g++
CCFLAGS = -W -g3 -std=c++14

PATH_SRC = ./../src
PATH_BIN = ./../bin/Routing_Algorithm_Binaries

PATH_GEN = ./General
PATH_CLS = ./Clustering

NAME_DEP = $(addprefix $(PATH_BIN)/, student.o vector2.o cluster.o main.o)

VEC2_DEP = $(addprefix $(PATH_GEN)/, vector2.hpp vector2.cpp)
SDNT_DEP = $(addprefix $(PATH_GEN)/, vector2.hpp student.hpp student.cpp)
CLST_DEP = $(addprefix $(PATH_GEN)/, student.hpp heap.hpp vector2.hpp) $(addprefix $(PATH_CLS)/, cluster.hpp cluster.cpp)
MAIN_DEP = $(addprefix $(PATH_CLS)/, cluster.hpp main.cpp)

all:
	@echo "\n>: Compiling executable "$(CCNAME)" :<"
	@echo "***"
	mkdir -p $(PATH_BIN)
	make $(PATH_BIN)/$(CCNAME)
	@echo "***\n"

$(PATH_BIN)/$(CCNAME): $(NAME_DEP)
	$(CC) $(CCFLAGS) $(NAME_DEP) -o $(PATH_BIN)/$(CCNAME)

$(PATH_BIN)/vector2.o: $(VEC2_DEP)
	$(CC) $(CCFLAGS) $(PATH_GEN)/vector2.cpp -c -o $(PATH_BIN)/vector2.o

$(PATH_BIN)/student.o: $(SDNT_DEP)
	$(CC) $(CCFLAGS) $(PATH_GEN)/student.cpp -c -o $(PATH_BIN)/student.o

$(PATH_BIN)/cluster.o: $(CLST_DEP)
	$(CC) $(CCFLAGS) $(PATH_CLS)/cluster.cpp -c -o $(PATH_BIN)/cluster.o

$(PATH_BIN)/main.o: $(MAIN_DEP)
	$(CC) $(CCFLAGS) $(PATH_CLS)/main.cpp -c -o $(PATH_BIN)/main.o
