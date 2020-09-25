# FuckSB

UEFI runtime driver

## Prerequisites

To build ia32 and x86_64 UEFI driver, the following tools are needed:

- make 
- gcc 
- git

For Ubuntu users, run the following command to install these packages:

```
sudo apt-get install git build-essential libc6-dev-i386
```

## Getting the source code

Execute the following command to obtain the source code:

```
git clone https://github.com/a1ive/fucksb
```

For convenience, the project relies on the gnu-efi library (but **not** on the gnu-efi compiler itself), so you need to initialize the git submodules:

```
git submodule init
git submodule update
```

## Compilation

If you're working in a cross-development environment, be sure to set
macro ARCH to the desired target architecture ("ia32" for x86, "x86_64" for
x86_64).  For convenience, this can also be done from
the make command line (e.g., "make ARCH=ia32").

**Compile x86_64 UEFI driver**

```
make ARCH=x86_64
```

**Compile ia32 UEFI driver**

```
make ARCH=ia32
```

## Usage

You need to load it using a1ive's GRUB 2.

1. Bypass certificate verification

   Secure boot prohibits the loading of unsigned UEFI drivers, so the driver needs to be signed and the key inserted before the driver can be loaded. See [Sakaki's EFI Install Guide](https://wiki.gentoo.org/wiki/User:Sakaki/Sakaki%27s_EFI_Install_Guide/Configuring_Secure_Boot) for details.

   You can also install a custom policy under GRUB2 to bypass security checks in order to load unsigned drivers. Simply execute the following code in GRUB2:

   ```
   sbpolicy --install
   ```

2. Enable/Disable Features (optional)

   FuckSB supports enabling or disabling some functions by reading the UEFI environment variable "FuckSBFlag". Just set the value of the environment variable before loading the driver.

   - Forced installation with or without secure boot on

     ```
     setenv -t uint8 FuckSBFlag 0x01
     ```

   - Install security policies to bypass certificate verification (same as sbpolicy command)

     ```
     setenv -t uint8 FuckSBFlag 0x02
     ```

   - Falsifying secure boot to `on` state

     ```
     setenv -t uint8 FuckSBFlag 0x04
     ```

   - Does not retain forged secure boot state after OS loading

     ```
     setenv -t uint8 FuckSBFlag 0x08
     ```

3. Load the driver

```
efiload -n /path/to/fucksb.efi
```

## Credits

- [shim](https://github.com/rhboot/shim), a first-stage UEFI bootloader.
- [EFI-memory](https://github.com/SamuelTulach/efi-memory), a proof-of-concept EFI runtime driver for reading and writing to virtual memory.