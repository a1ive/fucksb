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

### Enable/Disable Features

FuckSB supports enabling or disabling some functions by reading the UEFI environment variable "FuckSBFlag". Just set the value of the environment variable before loading the driver.

1. Forced installation with or without secure boot on

   ```
   setenv -t uint8 FuckSBFlag 0x01
   ```

2. Install security policies to bypass certificate verification

   ```
   setenv -t uint8 FuckSBFlag 0x02
   ```

3. Falsifying secure boot to `on` state

   ```
   setenv -t uint8 FuckSBFlag 0x04
   ```

4. Does not retain forged secure boot state after OS loading

   ```
   setenv -t uint8 FuckSBFlag 0x08
   ```

## Credits

- [shim](https://github.com/rhboot/shim), a first-stage UEFI bootloader.
- [EFI-memory](https://github.com/SamuelTulach/efi-memory), a proof-of-concept EFI runtime driver for reading and writing to virtual memory.