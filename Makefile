CFLAGS = $(shell pkg-config --cflags pluma) -shared -fPIC -Wno-deprecated -Wno-deprecated-declarations
LIBS = $(shell pkg-config --libs pluma)

# FIXME
PLUGINSDIR = /usr/lib/x86_64-linux-gnu/pluma/plugins/


.PHONY: all clean

all: iast/libiast.so length/liblength.so trailvisual/libtrailvisual.so

iast/libiast.so: iast/plugin.c
	$(QUIET_CC) $(CC) $^ -o $@ $(CFLAGS) $(LIBS) -liast

length/liblength.so: length/plugin.c
	$(QUIET_CC) $(CC) $^ -o $@ $(CFLAGS) $(LIBS)

trailvisual/libtrailvisual.so: trailvisual/plugin.c
	$(QUIET_CC) $(CC) $^ -o $@ $(CFLAGS) $(LIBS)

install:
	install -m 0755 iast/libiast.so $(PLUGINSDIR)
	install -m 0644 iast/iast.plugin $(PLUGINSDIR)
	install -m 0755 length/liblength.so $(PLUGINSDIR)
	install -m 0644 length/length.plugin $(PLUGINSDIR)
	install -m 0755 trailvisual/libtrailvisual.so $(PLUGINSDIR)
	install -m 0644 trailvisual/trailvisual.plugin $(PLUGINSDIR)

uninstall:
	rm -f $(PLUGINSDIR)/libiast.so
	rm -f $(PLUGINSDIR)/iast.plugin

clean:
	rm -f */*.so

ifndef V
QUIET_CC    = @echo "  CC     $@";
endif
