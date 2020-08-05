all: simple-upnpd
.PHONY: all clean

GUPNP := $(shell if pkg-config --exists gupnp-1.2 ; then echo gupnp-1.2; else echo gupnp-1.0; fi)

PKGS = glib-2.0 gobject-2.0 $(GUPNP) libsoup-2.4
override CFLAGS += ${shell pkg-config --cflags $(PKGS)} -Wall
override LDLIBS += ${shell pkg-config --libs $(PKGS)}

ifeq ($(GUPNP),gupnp-1.2)
override CFLAGS += -DGUPNP_1_2
endif

simple-upnpd: simple-upnpd.o

clean:
	rm -f simple-upnpd.o simple-upnpd

