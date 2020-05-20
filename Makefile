TARGET = fucksb.efi

export TOPDIR	:= $(shell pwd)/

include Make.rules

all: $(TARGET)

lib/lib.a lib/lib-efi.a: FORCE
	$(MAKE) -C lib $(notdir $@)

fucksb.so: lib/lib-efi.a

clean:
	rm -f *.o *.so *.efi
	$(MAKE) -C lib clean

FORCE:
