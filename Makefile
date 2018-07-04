
CCNAME = MMGP
CC = g++
CCFLAGS = -W -g3 -std=c++14

PATH_SRC = ./src
PATH_INC = ./include
PATH_BIN = ./bin

VEC2_DEP = $(PATH_INC)/vector2.hpp $(PATH_SRC)/vector2.cpp
SDNT_DEP = $(addprefix $(PATH_INC)/, vector2.hpp student.hpp) $(PATH_SRC)/student.cpp
CLST_DEP = $(addprefix $(PATH_INC)/, student.hpp heap.hpp vector2.hpp cluster.hpp) $(PATH_SRC)/cluster.cpp
MAIN_DEP = $(PATH_INC)/cluster.hpp $(PATH_SRC)/main.cpp

NAME_DEP = $(addprefix $(PATH_BIN)/, vector2.o student.o cluster.o main.o)

.PHONY: all
all:
	@echo "\n*** Compiling executable "$(PATH_BIN)/$(CCNAME)" ***"
	@echo "***"
	mkdir -p $(PATH_BIN)
	make $(PATH_BIN)/$(CCNAME)
	@echo "***\n"

$(PATH_BIN)/vector2.o: $(VEC2_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/vector2.cpp -c -o $(PATH_BIN)/vector2.o

$(PATH_BIN)/student.o: $(SDNT_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/student.cpp -c -o $(PATH_BIN)/student.o

$(PATH_BIN)/cluster.o: $(CLST_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/cluster.cpp -c -o $(PATH_BIN)/cluster.o

$(PATH_BIN)/main.o: $(MAIN_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/main.cpp -c -o $(PATH_BIN)/main.o

$(PATH_BIN)/$(CCNAME): $(NAME_DEP)
	$(CC) $(CCFLAGS) $(NAME_DEP) -o $(PATH_BIN)/$(CCNAME)

.PHONY: clean
clean:
	@echo "\n*** Purging "$(PATH_BIN)" ***"
	@echo "***"
	rm -rvI $(PATH_BIN)
	@echo "***\n"
	