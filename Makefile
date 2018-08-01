
.PHONY: all
all:
	$(MAKE) -C ./c++

.PHONY: clean
clean:
	@echo "\n*** Purging "./bin" ***"
	@echo "***"
	rm -rvf ./bin
	@echo "***\n"
