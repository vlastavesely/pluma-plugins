CFLAGS = $(shell pkg-config --cflags pluma) -shared -fPIC -Wno-deprecated -Wno-deprecated-declarations
LIBS = $(shell pkg-config --libs pluma)

# FIXME
PLUGINSDIR = /usr/lib/x86_64-linux-gnu/pluma/plugins/


.PHONY: all clean

all: iast/libiast.so length/liblength.so trailvisual/libtrailvisual.so lipsum/liblipsum.so

iast/libiast.so: iast/plugin.c
	$(QUIET_CC) $(CC) $^ -o $@ $(CFLAGS) $(LIBS) -liast

length/liblength.so: length/plugin.c
	$(QUIET_CC) $(CC) $^ -o $@ $(CFLAGS) $(LIBS)

trailvisual/libtrailvisual.so: trailvisual/plugin.c
	$(QUIET_CC) $(CC) $^ -o $@ $(CFLAGS) $(LIBS)

lipsum/liblipsum.so: lipsum/plugin.c
	$(QUIET_CC) $(CC) $^ -o $@ $(CFLAGS) $(LIBS)

install:
	install -m 0755 iast/libiast.so $(PLUGINSDIR)
	install -m 0644 iast/iast.plugin $(PLUGINSDIR)
	install -m 0755 length/liblength.so $(PLUGINSDIR)
	install -m 0644 length/length.plugin $(PLUGINSDIR)
	install -m 0755 trailvisual/libtrailvisual.so $(PLUGINSDIR)
	install -m 0644 trailvisual/trailvisual.plugin $(PLUGINSDIR)
	install -m 0755 lipsum/liblipsum.so $(PLUGINSDIR)
	install -m 0644 lipsum/lipsum.plugin $(PLUGINSDIR)
	mkdir -p /usr/share/pluma/plugins/lipsum
	install -m 0644 lipsum/lipsum.txt /usr/share/pluma/plugins/lipsum

uninstall:
	rm -rf /usr/share/pluma/plugins/lipsum
	rm -f $(PLUGINSDIR)/libiast.so
	rm -f $(PLUGINSDIR)/iast.plugin

clean:
	rm -f */*.so

ifndef V
QUIET_CC    = @echo "  CC     $@";
endif
