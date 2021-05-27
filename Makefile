.PHONY: release

.PHONY: debug
.PHONY: clean
.PHONY: coverage
.PHONY: coverage-build
.PHONY: install

release:
	maike2 --configfiles=maikeconfig2.json,maikeconfig2-rel.json --target-dir=__targets_rel

debug:
	maike2 --configfiles=maikeconfig2.json,maikeconfig2-dbg.json --target-dir=__targets_dbg

clean:
	rm -rf __targets_*

coverage: __targets_gcov/.coverage/coverage.html

coverage-build: maikeconfig.json maikeconfig-gcov.json
	maike --configfiles=maikeconfig.json,maikeconfig-gcov.json

__targets_gcov/.coverage/coverage.html: coverage-build ./coverage_collect.sh
	./coverage_collect.sh

DESTDIR?=""
PREFIX?="/usr"
install: release make_pkgconfig.sh
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/include/wad64
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/lib/pkgconfig
	mkdir -p $(DESTDIR)$(PREFIX)/man/man1
	mkdir -p $(DESTDIR)$(PREFIX)/man/man5
	mkdir -p $(DESTDIR)$(PREFIX)/share/mime/packages
	mkdir -p $(DESTDIR)$(PREFIX)/share/thumbnailers
	cp __targets_rel/bin/wad64 $(DESTDIR)$(PREFIX)/bin/wad64
	cp bin/wad64.1.man $(DESTDIR)$(PREFIX)/man/man1/wad64.1
	cp doc/wad64.5.man $(DESTDIR)$(PREFIX)/man/man5/wad64.5
	cp __targets_rel/lib/libwad64.a $(DESTDIR)$(PREFIX)/lib/libwad64.a
	cd lib && find -name '*.hpp' \
	    | while read in; do grep -v '^//@' "$$in" \
	    > $(DESTDIR)$(PREFIX)/include/wad64/$$in; done
	cp xdg/application-x-wad64.xml $(DESTDIR)$(PREFIX)/share/mime/packages

	./make_pkgconfig.sh $(PREFIX) $(DESTDIR)$(PREFIX)/lib/pkgconfig/wad64.pc
	./make_thumbnailer.sh $(PREFIX) $(DESTDIR)$(PREFIX)/share/thumbnailers/wad64.thumbnailer
