CFLAGS=-c -fno-stack-protector -fPIC -fshort-wchar -mno-red-zone -I /usr/include/efi -I /usr/include/efi/x86_64 -DEFI_FUNCTION_WRAPPER
LDFLAGS=-nostdlib -znocombreloc -T /usr/lib64/elf_x86_64_efi.lds -shared -Bsymbolic -L /usr/lib64 -l:libgnuefi.a -l:libefi.a
OBJCFLAGS=-j .text -j sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-x86_64

C=WinClear.c
O=$(C:.c=.o)

all: EFI

debug: debugEFI

install: installEFI

clean: cleanEFI



EFI: $(C) WinClear.efi

debugEFI: CFLAGS += -D_DEBUG
debugEFI: disk.hdd

installEFI: WinClear.efi
	-mkdir -p /boot/EFI/WinClear
	cp WinClear.efi /boot/EFI/WinClear/WinClear.efi
	@echo "Please replace the Windows Boot Manager path in your bootloader with EFI\\WinClear\\WinClear.efi"

cleanEFI:	
	-umount tmp
	-rm -r *.o *.so *.efi *.img *.hdd tmp



.c.o:
	gcc $< $(CFLAGS) -o $(@)

WinClear.efi: $(O)	
	ld $(O) /usr/lib64/crt0-efi-x86_64.o $(LDFLAGS) -o WinClear.so
	objcopy $(OBJCFLAGS) WinClear.so $(@)

.PHONY: disk.hdd
disk.hdd: WinClear.efi
	-mkdir tmp
	dd if=/dev/zero of=fat.img bs=1M count=10
	mkfs.fat -F16 fat.img
	mount fat.img tmp
	-mkdir -p tmp/EFI/BOOT
	cp WinClear.efi tmp/EFI/BOOT/BOOTx64.efi
	sleep 0.1
	umount tmp
	mkgpt -o disk.hdd --part fat.img --type system
	chmod 776 disk.hdd
