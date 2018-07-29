
.PHONY: all
all:
	$(MAKE) -C ./c++

.PHONY: debug
debug:
	$(MAKE) DEFLAGS="-D __DEBUG__" -C ./c++

.PHONY: clean
clean:
	@echo "\n*** Purging "./bin" ***"
	@echo "***"
	rm -rvf ./bin
	@echo "***\n"
