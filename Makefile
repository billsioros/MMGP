
PATH_BIN = ./Routing_Algorithm_Binaries/

.PHONY: all
all:
	$(MAKE) -C ./src/

.PHONY: clean
clean:
	rm -rI $(PATH_BIN)
