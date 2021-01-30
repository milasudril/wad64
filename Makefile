.PHONY: release

.PHONY: debug
.PHONY: clean
.PHONY: coverage
.PHONY: coverage-build

release:
	maike --configfiles=maikeconfig.json,maikeconfig-rel.json

debug:
	maike --configfiles=maikeconfig.json,maikeconfig-dbg.json

clean:
	rm -rf __targets_*

coverage: __targets_gcov/.coverage/coverage.html

coverage-build: maikeconfig.json maikeconfig-gcov.json
	maike --configfiles=maikeconfig.json,maikeconfig-gcov.json

__targets_gcov/.coverage/coverage.html: coverage-build ./coverage_collect.sh
	./coverage_collect.sh