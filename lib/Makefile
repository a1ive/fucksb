OBJECTS := sbpolicy.o getvar.o

LIBFILES = $(OBJECTS)
EFILIBFILES = $(patsubst %.o,%.efi.o,$(OBJECTS))

include ../Make.rules

lib.a: $(LIBFILES)
lib-efi.a: $(EFILIBFILES)

clean:
	rm -f lib.a
	rm -f lib-efi.a
	rm -f *.o
