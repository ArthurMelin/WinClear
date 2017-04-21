##
## Makefile for WinClear in /home/arthur.melin/Code/WinClear
##
## Made by Arthur Melin
##
## Started on  Fri Apr 21 12:07:15 2017 Arthur Melin
## Last update Fri Apr 21 17:01:16 2017 Arthur Melin
##

################
# Configuration:
ARCH		=	x86_64
BOOT_PATH	=	"L\"\\\\EFI\\\\Microsoft\\\\Boot\\\\bootmgfw.efi\""
################

NAME		=	WinClear-$(ARCH).efi
NAMESO		=	WinClear-$(ARCH).so

SRCS		=	WinClear.c
INCS		=	WinClear.h
OBJS		=	$(SRCS:.c=.o)


SRCSDIR		=	src
INCSDIR		=	include
OBJSDIR		=	obj

EFI_INCSDIRS	=	/usr/include/efi	\
			/usr/include/efi/$(ARCH)
EFI_LIBSDIRS	=	/usr/lib
EFI_LIBS	=	-lgnuefi -lefi
EFI_OBJS	=	/usr/lib/crt0-efi-$(ARCH).o

CC		=	gcc
LD		=	ld
OBJCP		=	objcopy

CFLAGS		=	-c -W -Wall -Wextra				\
			$(addprefix -I,$(INCSDIR) $(EFI_INCSDIRS))	\
			-DEFI_FUNCTION_WRAPPER				\
			-DBOOT_PATH=$(BOOT_PATH)			\
			-ffreestanding					\
			-fno-stack-protector				\
			-fpic						\
			-fshort-wchar					\
			-mno-red-zone
LDFLAGS		=	-shared				\
			$(addprefix -L,$(EFI_LIBSDIRS))	\
			-Telf_$(ARCH)_efi.lds		\
			$(EFI_LIBS)			\
			-Bsymbolic			\
			-nostdlib			\
			-zcombreloc
OBJCPFLAGS	=	-j .text	\
			-j .data	\
			-j .sdata	\
			-j .reloc	\
			-j .rel		\
			-j .rela	\
			-j .dynamic	\
			-j .dynsym	\
			--target=efi-app-$(ARCH)

RM		=	rm -rf
MKDIR		=	mkdir

all: $(NAME)

$(NAME): $(NAMESO)
	$(OBJCP) $(OBJCPFLAGS) $< $@

$(NAMESO): $(addprefix $(OBJSDIR)/,$(OBJS)) $(EFI_OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

$(OBJSDIR)/%.o: $(SRCSDIR)/%.c $(addprefix $(INCSDIR)/,$(INCS)) | $(OBJSDIR)
	$(CC) $< $(CFLAGS) -o $@

$(OBJSDIR):
	$(MKDIR) $@

clean:
	$(RM) $(OBJSDIR)
	$(RM) $(NAMESO)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
