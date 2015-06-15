
FLAGS=$(shell pkg-config --cflags libnautilus-extension glib-2.0 gtk+-3.0)
LIBS=$(shell pkg-config --cflags libnautilus-extension  glib-2.0 gtk+-3.0)

all: crear_lanzador.so

view:
	@echo PATH=$(PATH)
	@echo FLAGS=$(FLAGS)
	@echo LIBS=$(LIBS)


crear_lanzador.so:
	gcc -c crear_lanzador.c -o crear_lanzador.o -fPIC $(FLAGS)
	gcc -shared crear_lanzador.o -o crear_lanzador.so $(LIBS)


clean:
	rm -f crear_lanzador.so crear_lanzador.o


install: crear_lanzador.so
	# install
	install -d $(DESTDIR)/usr/bin/
	install -d $(DESTDIR)/usr/lib/nautilus/extensions-3.0/

	install -m 755 crear_lanzador     $(DESTDIR)/usr/bin/
	install -m 644 crear_lanzador.so  $(DESTDIR)/usr/lib/nautilus/extensions-3.0/
