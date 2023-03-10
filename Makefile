CFLAGS = $(shell pkg-config --cflags pluma) -shared -fPIC # -Wno-deprecated -Wno-deprecated-declarations
LIBS = $(shell pkg-config --libs pluma)

# FIXME
PLUGINSDIR = /usr/lib/x86_64-linux-gnu/pluma/plugins/


.PHONY: all clean

all: iast/libiast.so

iast/libiast.so: iast/plugin.c
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

install:
	install -m 0755 iast/libiast.so $(PLUGINSDIR)
	install -m 0644 iast/iast.plugin $(PLUGINSDIR)

uninstall:
	rm -f $(PLUGINSDIR)/libiast.so
	rm -f $(PLUGINSDIR)/iast.plugin

clean:
	rm -f */*.so
