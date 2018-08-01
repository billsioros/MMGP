
.PHONY: all
all:
	$(MAKE) -C ./cpp

.PHONY: clean
clean:
	@echo "\n*** Purging "./bin" ***"
	@echo "***"
	rm -rvf ./bin
	@echo "***\n"
