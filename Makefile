
PATH_BIN = ./bin
PATH_SRC = ./src

.PHONY: all
all:
	make Routing

.PHONY: Routing
Routing:
	$(MAKE) -C $(PATH_SRC)/Routing_Algorithm/

.PHONY: clean
clean:
	@echo "***>! Purging !<***"
	rm -rvI $(PATH_BIN)
