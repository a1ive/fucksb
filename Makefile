TARGET = fucksb.efi

export TOPDIR	:= $(shell pwd)/

include Make.rules

all: $(GNUEFI_DIR)/$(ARCH)/lib/libefi.a $(TARGET)

$(GNUEFI_DIR)/$(ARCH)/lib/libefi.a:
	$(MAKE) -C$(GNUEFI_DIR) ARCH=$(ARCH)

lib/lib.a lib/lib-efi.a: FORCE
	$(MAKE) -C lib $(notdir $@)

fucksb.so: lib/lib-efi.a

clean:
	rm -f *.o *.so *.efi
	$(MAKE) -C lib clean
	$(MAKE) -C gnu-efi clean
	rm -rf $(GNUEFI_DIR)/x86_64 $(GNUEFI_DIR)/ia32

FORCE:
