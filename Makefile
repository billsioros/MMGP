
PATH_BIN = ./bin
PATH_SRC = ./src

.PHONY: all
all:
	make routing

.PHONY: routing
routing:
	$(MAKE) -C $(PATH_SRC)/Routing_Algorithm/

.PHONY: clean
clean:
	rm -rvI $(PATH_BIN)
