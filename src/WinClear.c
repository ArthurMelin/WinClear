/*
** WinClear.c for WinClear in /home/arthur.melin/Code/WinClear
**
** Made by Arthur Melin
**
** Started on  Fri Apr 21 12:31:23 2017 Arthur Melin
** Last update Fri Apr 21 20:02:28 2017 Arthur Melin
*/

#include <WinClear.h>

static ACPI_RSDP		*FindAcpiRsdp(EFI_SYSTEM_TABLE *SystemTable)
{
  UINTN				i;
  EFI_CONFIGURATION_TABLE	*ConfigurationTable;

  i = 0;
  ConfigurationTable = SystemTable->ConfigurationTable;
  while (i < SystemTable->NumberOfTableEntries)
    {
      if (!CompareGuid(&ConfigurationTable[i].VendorGuid,
		       &Acpi20TableGuid))
	return (ConfigurationTable[i].VendorTable);
      i++;
    }
  Print(L"Couldn't find ACPI root table (RSDP) in the EFI configuration table\n");
  return (NULL);
}

static ACPI_BGRT		*FindAcpiBgrt(ACPI_RSDP *AcpiRsdp)
{
  UINTN				SdtEntryAddr;
  ACPI_SDT_HEADER		*SdtHeader;
  void				*EntryPtr;

  if (AcpiRsdp->Revision != 1 && AcpiRsdp->Revision != 2)
    {
      Print(L"Unsupported ACPI root table (RSDP) revision\n");
      return (NULL);
    }
  SdtHeader = (ACPI_SDT_HEADER *)(UINTN)
    (AcpiRsdp->Revision == 1 ? AcpiRsdp->RsdtAddr : AcpiRsdp->XsdtAddr);
  SdtEntryAddr = (UINTN)SdtHeader + sizeof(ACPI_SDT_HEADER);
  while (SdtEntryAddr < (UINTN)SdtHeader + SdtHeader->Length)
    {
      EntryPtr = AcpiRsdp->Revision == 1 ?
	(void *)(UINTN)*(UINT32 *)SdtEntryAddr :
	(void *)(UINTN)*(UINT64 *)SdtEntryAddr;
      if (!CompareMem(((ACPI_SDT_HEADER *)EntryPtr)->Signature, "BGRT", 4))
	return ((ACPI_BGRT *)EntryPtr);
      SdtEntryAddr += AcpiRsdp->Revision == 1 ? 4 : 8;
    }
  Print(L"Couldn't find the ACPI boot graphics table (BGRT)\n");
  return (NULL);
}

static int	ClearBG(ACPI_BGRT *AcpiBgrt)
{
  BMP_HEADER	*BmpHeader;

  if (AcpiBgrt->Version != 1)
    {
      Print(L"Unsupported ACPI boot graphics table (BGRT) version\n");
      return (1);
    }
  if (AcpiBgrt->ImageType != 0)
    {
      Print(L"Unsupported boot graphics image type (not bitmap)\n");
      return (1);
    }
  BmpHeader = (BMP_HEADER *)(UINTN)AcpiBgrt->ImageAddr;
  if (CompareMem(BmpHeader->Signature, "BM", 2))
    {
      Print(L"Invalid bitmap header signature\n");
      return (1);
    }
  ZeroMem(BmpHeader, BmpHeader->Size);
  return (0);
}

static int		LoadOsLoader(EFI_HANDLE ImageHandle,
				     EFI_SYSTEM_TABLE *SystemTable,
				     EFI_HANDLE *OsImage)
{
  EFI_LOADED_IMAGE	*LoadedImage;
  EFI_DEVICE_PATH	*Path;

  if (uefi_call_wrapper(SystemTable->BootServices->OpenProtocol, 6,
			ImageHandle, &LoadedImageProtocol, &LoadedImage,
			ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL) ||
      !(Path = FileDevicePath(LoadedImage->DeviceHandle, BOOT_PATH)))
    {
      Print(L"Failed to get boot device path\n");
      return (1);
    }
  uefi_call_wrapper(SystemTable->BootServices->CloseProtocol, 4,
		    ImageHandle, &LoadedImageProtocol, ImageHandle, NULL);
  if (uefi_call_wrapper(SystemTable->BootServices->LoadImage, 6,
			FALSE, ImageHandle, Path, NULL, 0, OsImage))
    {
      Print(L"Failed to load OS bootloader image\n");
      FreePool(Path);
      return (1);
    }
  FreePool(Path);
  return (0);
}

EFI_STATUS EFIAPI	efi_main(EFI_HANDLE ImageHandle,
				 EFI_SYSTEM_TABLE *SystemTable)
{
  ACPI_RSDP		*AcpiRsdp;
  ACPI_BGRT		*AcpiBgrt;
  EFI_HANDLE		OsImage;
  EFI_STATUS		Ret;

  InitializeLib(ImageHandle, SystemTable);
  if (!(AcpiRsdp = FindAcpiRsdp(SystemTable)) ||
      !(AcpiBgrt = FindAcpiBgrt(AcpiRsdp)) ||
      ClearBG(AcpiBgrt))
    {
      Print(L"Starting OS in 3 seconds...\n");
      uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 3000000);
    }
  if (LoadOsLoader(ImageHandle, SystemTable, &OsImage))
    {
      uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 5000000);
      return (EFI_LOAD_ERROR);
    }
  Ret = uefi_call_wrapper(SystemTable->BootServices->StartImage, 3,
			  OsImage, NULL, NULL);
  uefi_call_wrapper(SystemTable->BootServices->UnloadImage, 1, OsImage);
  return (Ret);
}
