LIB1     = libvmxpi_hal_cpp.so

LIB      = $(LIB1)

ALL     = $(LIB)

package = vmxpi_hal_cpp
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include/$(package)
libdir = $(prefix)/lib/$(package)
mandir = $(prefix)/man

install:
	install -m 0755 -d                $(DESTDIR)$(includedir)
	install -m 0644 include/*         $(DESTDIR)$(includedir)
	install -m 0755 -d                $(DESTDIR)$(libdir)
	install -m 0755 Debug/$(LIB1)     $(DESTDIR)$(libdir)
	ldconfig $(libdir)

uninstall:
	rm -r -f $(DESTDIR)$(includedir)
	rm -r -f $(DESTDIR)$(libdir)
	ldconfig
