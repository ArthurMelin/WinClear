# WinClear
EFI application that clears the firmware logo then start Windows.

## Pre-requisites
Before building this application, you will need to install GCC and GNU-EFI with your distribution's package manager. You will also need to download and build [makegpt](http://www.tysos.org/redmine/projects/tysos/wiki/EFI) (the archive is also available in this repository in case the link doesn't work anymore) and install FAT filesystem support if you want to test the application in an emulator.

The Makefile assumes the includes and libs paths for Arch Linux, correct them if necessary if you use a different distribution.

The install target of the Makefile also assumes that your EFI System Partition is mounted on /boot, correct that too if your ESP is mounted elsewhere.

Also check that the Windows Boot Manager image file is located at `<path-to-esp>/EFI/Microsoft/Boot/bootmgfw.efi`.

## Building and installing
To build and install the application, simply type `sudo make install` in the terminal (root privileges are needed to copy the .efi file to the ESP).

To use this application, edit your bootloader's config to use `ESP:\EFI\WinClear\WinClear.efi` instead of `ESP:\EFI\Microsoft\Boot\bootmgfw.efi` to start Windows.
