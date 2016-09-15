all: simple-upnpd
.PHONY: all clean

PKGS = glib-2.0 gobject-2.0 gupnp-1.0 libsoup-2.4
override CFLAGS += ${shell pkg-config --cflags $(PKGS)} -Wall
override LDLIBS += ${shell pkg-config --libs $(PKGS)}

simple-upnpd: simple-upnpd.o

clean:
	rm -f upnp-announced.o upnp-announced

