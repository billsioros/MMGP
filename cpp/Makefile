
CCNAME = MMGP
CC = g++
CCFLAGS = -W -g3 -std=c++17

PATH_SRC = ./src
PATH_INC = ./include
PATH_BIN = ./../bin

PATH_SQL_INC = ./SQLiteCpp/include
PATH_SQL_SRC = ./SQLiteCpp/src

PATH_JSN_INC = ./nlohmann

TIME_DEP = $(PATH_INC)/timewindow.hpp $(PATH_SRC)/timewindow.cpp
VEC2_DEP = $(PATH_INC)/vector2.hpp $(PATH_SRC)/vector2.cpp
MNGR_DEP = $(PATH_SQL_INC)/Database.h $(PATH_JSN_INC)/json.hpp $(addprefix $(PATH_INC)/, timewindow.hpp vector2.hpp manager.hpp log.hpp) $(PATH_SRC)/manager.cpp
LOG__DEP = $(addprefix $(PATH_INC)/, log.hpp formatter.hpp) $(PATH_SRC)/log.cpp
MAIN_DEP = $(PATH_JSN_INC)/json.hpp $(addprefix $(PATH_SQL_INC)/, Database.h Statement.h) $(addprefix $(PATH_INC)/, cmeans.hpp manager.hpp tsp.hpp annealing.hpp log.hpp) $(PATH_SRC)/main.cpp

BACK_DEP = $(addprefix $(PATH_SQL_INC)/, Backup.h Exception.h) $(PATH_SQL_SRC)/Backup.cpp
CLMN_DEP = $(PATH_SQL_INC)/Column.h $(PATH_SQL_SRC)/Column.cpp
DATA_DEP = $(addprefix $(PATH_SQL_INC)/, Statement.h Assertion.h Exception.h) $(PATH_SQL_SRC)/Database.cpp
EXCP_DEP = $(PATH_SQL_INC)/Exception.h $(PATH_SQL_SRC)/Exception.cpp
STMT_DEP = $(addprefix $(PATH_SQL_INC)/, Statement.h Database.h Column.h Assertion.h Exception.h) $(PATH_SQL_SRC)/Statement.cpp
TRNS_DEP = $(addprefix $(PATH_SQL_INC)/, Transaction.h Database.h Assertion.h) $(PATH_SQL_SRC)/Transaction.cpp

NAME_DEP = $(addprefix $(PATH_BIN)/, timewindow.o vector2.o manager.o main.o log.o Backup.o Column.o Database.o Exception.o Statement.o Transaction.o)

.PHONY: all
all:
	@echo "\n*** Compiling executable "$(CCNAME)" ***"
	@echo "***"
	mkdir -p $(PATH_BIN)/debug
	make $(PATH_BIN)/$(CCNAME)
	@echo "***\n"

# MMGP:

$(PATH_BIN)/$(CCNAME): $(NAME_DEP)
	$(CC) $(DEFLAGS) $(CCFLAGS) $(NAME_DEP) -lsqlite3 -o $(PATH_BIN)/$(CCNAME)

$(PATH_BIN)/timewindow.o: $(TIME_DEP)
	$(CC) -I $(PATH_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SRC)/timewindow.cpp -c -o $(PATH_BIN)/timewindow.o

$(PATH_BIN)/vector2.o: $(VEC2_DEP)
	$(CC) -I $(PATH_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SRC)/vector2.cpp -c -o $(PATH_BIN)/vector2.o

$(PATH_BIN)/manager.o: $(MNGR_DEP)
	$(CC) -I $(PATH_INC) -I $(PATH_SQL_INC) -I $(PATH_JSN_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SRC)/manager.cpp -c -o $(PATH_BIN)/manager.o

$(PATH_BIN)/log.o: $(LOG__DEP)
	$(CC) -I $(PATH_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SRC)/log.cpp -c -o $(PATH_BIN)/log.o

$(PATH_BIN)/main.o: $(MAIN_DEP)
	$(CC) -I $(PATH_JSN_INC) -I $(PATH_INC) -I $(PATH_SQL_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SRC)/main.cpp -c -o $(PATH_BIN)/main.o

# SQLite3 Wrapper:

$(PATH_BIN)/Backup.o: $(BACK_DEP)
	$(CC) -I $(PATH_SQL_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SQL_SRC)/Backup.cpp -c -o $(PATH_BIN)/Backup.o

$(PATH_BIN)/Column.o: $(CLMN_DEP)
	$(CC) -I $(PATH_SQL_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SQL_SRC)/Column.cpp -c -o $(PATH_BIN)/Column.o

$(PATH_BIN)/Database.o: $(DATA_DEP)
	$(CC) -I $(PATH_SQL_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SQL_SRC)/Database.cpp -c -o $(PATH_BIN)/Database.o

$(PATH_BIN)/Exception.o: $(EXCP_DEP)
	$(CC) -I $(PATH_SQL_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SQL_SRC)/Exception.cpp -c -o $(PATH_BIN)/Exception.o

$(PATH_BIN)/Statement.o: $(STMT_DEP)
	$(CC) -I $(PATH_SQL_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SQL_SRC)/Statement.cpp -c -o $(PATH_BIN)/Statement.o

$(PATH_BIN)/Transaction.o: $(TRNS_DEP)
	$(CC) -I $(PATH_SQL_INC) $(DEFLAGS) $(CCFLAGS) $(PATH_SQL_SRC)/Transaction.cpp -c -o $(PATH_BIN)/Transaction.o
