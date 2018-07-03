
PATH_BIN = ./bin
PATH_SRC = ./src

.PHONY: all
all:
	$(MAKE) -C $(PATH_SRC)

.PHONY: clean
clean:
	@echo "***>! Purging binaries !<***"
	rm -rvI $(PATH_BIN)
	