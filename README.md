# WinClear
WinClear is a (U)EFI application that can be used to clear your motherboard logo during boot.

This can be useful if your computer motherboard firmware displays a full-screen logo and doesn't clear it before starting your OS (e.g. Windows) causing some ugly combinations of the logo and your OS boot animation overlaying.

## Building
WinClear is meant to be built on a UNIX-like system, but if you use Windows, maybe you will be able to build using MinGW or Bash on Windows (WSL).

To build WinClear, you will first need to install the following tools and packages using your system package manager:
* make
* gcc
* binutils
* gnu-efi-libs

Then, you need to setup the Makefile: by default, it will try building for the x86_64 architecture and will setup WinClear to try booting the Windows bootloader (`\EFI\Microsoft\Boot\bootmgfw.efi`).  
If your computer runs on a different architecture or you want WinClear to start a different .efi file, simply edit the ARCH and BOOT_PATH variables at the top of the Makefile.  
Make sure that you write the boot path like it is by default, from the EFI boot partition (ESP) root directory, and by escaping each `\` twice (-> `\\\\`).

Finally, use the `make` command while in the repository top directory to build the .efi binary.

## Installing
To install WinClear, simply copy the .efi binary you built to your EFI boot partition and setup add WinClear to your bootloader (e.g. GRUB) boot options.
