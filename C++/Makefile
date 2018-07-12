
CCNAME = MMGP
CC = g++
CCFLAGS = -W -g3 -std=c++14

PATH_SRC = ./src
PATH_INC = ./include
PATH_BIN = ./bin

VEC2_DEP = $(PATH_INC)/vector2.hpp $(PATH_SRC)/vector2.cpp
UTIL_DEP = $(addprefix $(PATH_INC)/, vector2.hpp utility.hpp) $(PATH_SRC)/utility.cpp
SDNT_DEP = $(addprefix $(PATH_INC)/, vector2.hpp student.hpp) $(PATH_SRC)/student.cpp
CLST_DEP = $(addprefix $(PATH_INC)/, student.hpp heap.hpp cluster.hpp) $(PATH_SRC)/cluster.cpp
MAIN_DEP = $(addprefix $(PATH_INC)/, Database.h Statement.h cluster.hpp student.hpp utility.hpp) $(PATH_SRC)/main.cpp
BACK_DEP = $(addprefix $(PATH_INC)/, Backup.h Exception.h) $(PATH_SRC)/Backup.cpp
CLMN_DEP = $(PATH_INC)/Column.h $(PATH_SRC)/Column.cpp
DATA_DEP = $(addprefix $(PATH_INC)/, Statement.h Assertion.h Exception.h) $(PATH_SRC)/Database.cpp
EXCP_DEP = $(PATH_INC)/Exception.h $(PATH_SRC)/Exception.cpp
STMT_DEP = $(addprefix $(PATH_INC)/, Statement.h Database.h Column.h Assertion.h Exception.h) $(PATH_SRC)/Statement.cpp
TRNS_DEP = $(addprefix $(PATH_INC)/, Transaction.h Database.h Assertion.h) $(PATH_SRC)/Transaction.cpp

NAME_DEP = $(addprefix $(PATH_BIN)/, utility.o vector2.o student.o cluster.o main.o Backup.o Column.o Database.o Exception.o Statement.o Transaction.o)

.PHONY: all
all:
	@echo "\n*** Compiling executable "$(CCNAME)" ***"
	@echo "***"
	mkdir -p $(PATH_BIN)
	make $(PATH_BIN)/$(CCNAME)
	@echo "***\n"

.PHONY: clean
clean:
	@echo "\n*** Purging "$(PATH_BIN)" ***"
	@echo "***"
	rm -rvf $(PATH_BIN)
	@echo "***\n"

# MMGP:

$(PATH_BIN)/$(CCNAME): $(NAME_DEP)
	$(CC) $(CCFLAGS) $(NAME_DEP) -lsqlite3 -o $(PATH_BIN)/$(CCNAME)

$(PATH_BIN)/vector2.o: $(VEC2_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/vector2.cpp -c -o $(PATH_BIN)/vector2.o

$(PATH_BIN)/utility.o: $(UTIL_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/utility.cpp -c -o $(PATH_BIN)/utility.o

$(PATH_BIN)/student.o: $(SDNT_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/student.cpp -c -o $(PATH_BIN)/student.o

$(PATH_BIN)/cluster.o: $(CLST_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/cluster.cpp -c -o $(PATH_BIN)/cluster.o

$(PATH_BIN)/main.o: $(MAIN_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/main.cpp -c -o $(PATH_BIN)/main.o

# SQLite3 Wrapper:

$(PATH_BIN)/Backup.o: $(BACK_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/Backup.cpp -c -o $(PATH_BIN)/Backup.o

$(PATH_BIN)/Column.o: $(CLMN_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/Column.cpp -c -o $(PATH_BIN)/Column.o

$(PATH_BIN)/Database.o: $(DATA_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/Database.cpp -c -o $(PATH_BIN)/Database.o

$(PATH_BIN)/Exception.o: $(EXCP_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/Exception.cpp -c -o $(PATH_BIN)/Exception.o

$(PATH_BIN)/Statement.o: $(STMT_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/Statement.cpp -c -o $(PATH_BIN)/Statement.o

$(PATH_BIN)/Transaction.o: $(TRNS_DEP)
	$(CC) -I $(PATH_INC) $(CCFLAGS) $(PATH_SRC)/Transaction.cpp -c -o $(PATH_BIN)/Transaction.o
