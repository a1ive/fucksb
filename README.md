# FuckSB

UEFI runtime driver

## Prerequisites

To build ia32 and x86_64 UEFI driver, the following tools are needed:

- make 

- gnu-efi 

- gcc 

## Compilation

If you're working in a cross-development environment, be sure to set
macro ARCH to the desired target architecture ("ia32" for x86, "x86_64" for
x86_64).  For convenience, this can also be done from
the make command line (e.g., "make ARCH=ia32").

**x86_64 UEFI driver**

```
make ARCH=x86_64
```

**ia32 UEFI driver**

```
make ARCH=ia32
```

## Usage

You need to load it using UEFI shell or a1ive's GRUB 2.

### Using UEFI shell

```
load fucksb.efi
```

### Using a1ive's GRUB 2

```
efiload -n /path/to/fucksb.efi
```

