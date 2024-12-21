all: simple-upnpd
.PHONY: all clean

GUPNP := $(shell pkg-config --list-all | sed -n 's/^\(gupnp-[0-9.]\+\).*/\1/p')
SOUP := $(shell pkg-config --list-all | sed -ne 's/^\(libsoup-[0-9.]\+\).*/\1/p')
SOUP :=  $(firstword $(SOUP))

#$(info gupnp is $(GUPNP))
#$(info soup is $(SOUP))

PKGS = glib-2.0 gobject-2.0 $(GUPNP) $(SOUP)

override CFLAGS += $(shell pkg-config --cflags $(PKGS)) -Wall
ifneq ($(.SHELLSTATUS), 0)
  $(error pkg-config failed)
endif

EXTRA_LDLIBS != pkg-config --libs $(PKGS)
ifneq ($(.SHELLSTATUS), 0)
  $(error pkg-config failed)
endif

override LDLIBS += ${shell pkg-config --libs $(PKGS)}
ifneq ($(.SHELLSTATUS), 0)
  $(error pkg-config failed)
endif

GUPNP_VERSION := $(subst gupnp-, , $(subst ., ,$(GUPNP)))
GUPNP_MAJOR = $(word 1,$(GUPNP_VERSION))
GUPNP_MINOR = $(word 2,$(GUPNP_VERSION))
override CFLAGS += -DGUPNP_VERSION_MAJOR=$(GUPNP_MAJOR) -DGUPNP_VERSION_MINOR=$(GUPNP_MINOR) -DGUPNP_VERSION_MICRO=0

simple-upnpd: simple-upnpd.o

clean:
	rm -f simple-upnpd.o simple-upnpd

