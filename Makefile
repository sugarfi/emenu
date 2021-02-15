# emenu - dynamic menu
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c emenu.c util.c
OBJ = $(SRC:.c=.o)

all: options emenu

options:
	@echo emenu build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

.c.o:
	$(CC) -c $(CFLAGS) $<

config.h:
	cp config.def.h $@

$(OBJ): config.h config.mk drw.h

emenu: emenu.o drw.o util.o
	$(CC) -o $@ emenu.o drw.o util.o $(LDFLAGS)

clean:
	rm -f emenu $(OBJ) emenu-$(VERSION).tar.gz

dist: clean
	mkdir -p emenu-$(VERSION)
	cp LICENSE Makefile README arg.h config.def.h config.mk emenu.1\
		drw.h util.h emenu_path emenu_run stest.1 $(SRC)\
		emenu-$(VERSION)
	tar -cf emenu-$(VERSION).tar emenu-$(VERSION)
	gzip emenu-$(VERSION).tar
	rm -rf emenu-$(VERSION)

.PHONY: all options clean dist install uninstall
