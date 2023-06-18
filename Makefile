.PHONY: all
all: release dynlib

.PHONY: release
.PHONY: dynlib
.PHONY: debug
.PHONY: clean
.PHONY: coverage
.PHONY: coverage-build
.PHONY: install

release:
	maike2 --configfiles=maikeconfig2.json,maikeconfig2-rel.json --target-dir=__targets_rel

dynlib:
	maike2 --configfiles=maikeconfig2.json,maikeconfig-dynlib.json --target-dir=__targets_dynlib

debug:
	maike2 --configfiles=maikeconfig2.json,maikeconfig2-dbg.json --target-dir=__targets_dbg

clean:
	rm -rf __targets_*

coverage: __targets_gcov/.coverage/coverage.html

coverage-build:
	maike2 --configfiles=maikeconfig2.json,maikeconfig2-gcov.json --target-dir=__targets_gcov

__targets_gcov/.coverage/coverage.html: coverage-build ./coverage_collect.sh
	./coverage_collect.sh

DESTDIR?=""
PREFIX?="/usr"
install: release make_pkgconfig.sh
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/include/wad64
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/lib/pkgconfig
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man5
	mkdir -p $(DESTDIR)$(PREFIX)/share/mime/packages
	mkdir -p $(DESTDIR)$(PREFIX)/share/thumbnailers
	mkdir -p $(DESTDIR)$(PREFIX)/lib/python3/dist-packages
	cp __targets_rel/bin/wad64 $(DESTDIR)$(PREFIX)/bin/wad64
	cp bin/wad64.1.man $(DESTDIR)$(PREFIX)/share/man/man1/wad64.1
	cp doc/wad64.5.man $(DESTDIR)$(PREFIX)/share/man/man5/wad64.5
	cp __targets_rel/lib/libwad64.a $(DESTDIR)$(PREFIX)/lib/libwad64.a
	cd lib && find -name '*.hpp' \
	    | while read in; do grep -v '^//@' "$$in" \
	    > $(DESTDIR)$(PREFIX)/include/wad64/$$in; done
	cp xdg/application-x-wad64.xml $(DESTDIR)$(PREFIX)/share/mime/packages
	cp __targets_dynlib/lib/wad64py.so $(DESTDIR)$(PREFIX)/lib/python3/dist-packages/
	cp lib/wad64.py $(DESTDIR)$(PREFIX)/lib/python3/dist-packages/

	./make_pkgconfig.sh $(PREFIX) $(DESTDIR)$(PREFIX)/lib/pkgconfig/wad64.pc
	./make_thumbnailer.sh $(PREFIX) $(DESTDIR)$(PREFIX)/share/thumbnailers/wad64.thumbnailer
