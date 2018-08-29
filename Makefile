
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
	find ../ -name "Morning*.json" -delete
	find ../ -name "Noon*.json"    -delete
	find ../ -name "Study*.json"   -delete
	@echo "***"
