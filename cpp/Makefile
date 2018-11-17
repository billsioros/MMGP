
CCNAME = MMGP
CC = g++
CCFLAGS = -W -g3 -std=c++17

SRC = src
INC = include
LIB = lib
BIN = ../bin

PATH_SQL = SQLiteCpp
PATH_JSN = nlohmann

TIME_DEP = $(INC)/timewindow.hpp $(SRC)/timewindow.cpp
VEC2_DEP = $(INC)/vector2.hpp $(SRC)/vector2.cpp
MNGR_DEP = $(PATH_SQL)/$(INC)/Database.h $(PATH_JSN)/json.hpp $(addprefix $(INC)/, timewindow.hpp vector2.hpp manager.hpp log.hpp) $(SRC)/manager.cpp
LOG__DEP = $(addprefix $(INC)/, log.hpp formatter.hpp) $(SRC)/log.cpp
MAIN_DEP = $(PATH_JSN)/json.hpp $(addprefix $(PATH_SQL)/$(INC)/, Database.h Statement.h) $(addprefix $(INC)/, cmeans.hpp manager.hpp tsp.hpp annealing.hpp log.hpp) $(SRC)/main.cpp

NAME_DEP = $(addprefix $(BIN)/, timewindow.o vector2.o manager.o main.o log.o) $(PATH_SQL)/$(LIB)/libsqlcpp.so

.PHONY: all
all:
	@echo "\n*** Compiling executable "$(CCNAME)" ***"
	@echo "***"
	mkdir -p $(BIN)/debug
	make $(BIN)/$(CCNAME)
	@echo "***\n"

# MMGP:

$(BIN)/$(CCNAME): $(NAME_DEP)
	$(CC) -L$(PATH_SQL)/$(LIB)/libsqllcpp.so -I $(PATH_SQL)/$(INC) $(DEFLAGS) $(CCFLAGS) $(NAME_DEP) -ldl -pthread -o $(BIN)/$(CCNAME)

$(BIN)/timewindow.o: $(TIME_DEP)
	$(CC) -I $(INC) $(DEFLAGS) $(CCFLAGS) $(SRC)/timewindow.cpp -c -o $(BIN)/timewindow.o

$(BIN)/vector2.o: $(VEC2_DEP)
	$(CC) -I $(INC) $(DEFLAGS) $(CCFLAGS) $(SRC)/vector2.cpp -c -o $(BIN)/vector2.o

$(BIN)/manager.o: $(MNGR_DEP)
	$(CC) -I $(INC) -I $(PATH_SQL)/$(INC) -I $(PATH_JSN) $(DEFLAGS) $(CCFLAGS) $(SRC)/manager.cpp -c -o $(BIN)/manager.o

$(BIN)/log.o: $(LOG__DEP)
	$(CC) -I $(INC) $(DEFLAGS) $(CCFLAGS) $(SRC)/log.cpp -c -o $(BIN)/log.o

$(BIN)/main.o: $(MAIN_DEP)
	$(CC) -I $(PATH_JSN) -I $(INC) -I $(PATH_SQL)/$(INC) $(DEFLAGS) $(CCFLAGS) $(SRC)/main.cpp -c -o $(BIN)/main.o
