/*
** WinClear.h for WinClear in /home/arthur.melin/Code/WinClear
**
** Made by Arthur Melin
**
** Started on  Fri Apr 21 12:31:50 2017 Arthur Melin
** Last update Fri Apr 21 19:56:20 2017 Arthur Melin
*/

#ifndef WINCLEAR_H_
# define WINCLEAR_H_

#include <efi.h>
#include <efilib.h>

typedef struct __attribute__ ((packed))		_ACPI_RSDP
{
  CHAR8						Signature[8];
  UINT8						Checksum;
  CHAR8						OemId[6];
  UINT8						Revision;
  UINT32					RsdtAddr;
  UINT32					Length;
  UINT64					XsdtAddr;
  UINT8						ExtendedChecksum;
  CHAR8						Reserved[3];
}						ACPI_RSDP;

typedef struct __attribute__ ((packed))		_ACPI_SDT_HEADER
{
  CHAR8						Signature[4];
  UINT32					Length;
  UINT8						Revision;
  UINT8						Checksum;
  CHAR8						OemId[6];
  CHAR8						OemTableId[8];
  UINT32					OemRevision;
  UINT32					CreatorId;
  UINT32					CreatorRevision;
}						ACPI_SDT_HEADER;

typedef struct __attribute__ ((packed))		_ACPI_BGRT
{
  ACPI_SDT_HEADER				Header;
  UINT16					Version;
  UINT8						Status;
  UINT8						ImageType;
  UINT64					ImageAddr;
  UINT32					ImageOffsetX;
  UINT32					ImageOffsetY;
}						ACPI_BGRT;

typedef struct __attribute__ ((packed))		_BMP_HEADER
{
  CHAR8						Signature[2];
  UINT32					Size;
  CHAR8						Reserved;
  UINT32					ImageDataOffset;
}						BMP_HEADER;

static EFI_GUID		Acpi20TableGuid = ACPI_20_TABLE_GUID;

#endif /* !WINCLEAR_H_ */
