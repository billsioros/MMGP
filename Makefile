
.PHONY: all
all:
	$(MAKE) -C ./cpp

.PHONY: clean
clean:
	@echo "\n*** Purging binaries ***"
	@echo "***"
	rm -rvf ./bin
	@echo "***\n"

.PHONY: discard
discard:
	@echo "\n*** Purging data files ***"
	@echo "***"
	find . -name "*[0-9].json" -delete
	@echo "***"
