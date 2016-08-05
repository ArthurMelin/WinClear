#include <efi.h>
#include <efilib.h>

#ifdef _DEBUG
#define PrintDebug Print
#else
#define PrintDebug //
#endif

#define ExitOnError(COND,ERRORMSG,RET) if ((COND)) { Print((ERRORMSG), (RET)); uefi_call_wrapper(BS->Stall, 1, 10*1000*1000); return (RET); } //

typedef struct __attribute__((__packed__)) ACPI_RSDP
{
	CHAR8 Signature[8];
	UINT8 Checksum;
	CHAR8 OEMID[6];
	UINT8 Revision;
	UINT32 RsdtAddress;
	UINT32 Length;
	UINT64 XsdtAddress;
	UINT8 ExtendedChecksum;
	CHAR8 Reserved[3];
} ACPI_RSDP;

typedef struct __attribute__((__packed__)) ACPI_SDT_HEADER
{
	CHAR8 Signature[4];
	UINT32 Length;
	UINT8 Revision;
	UINT8 Checksum;
	CHAR8 OEMID[6];
	CHAR8 OEMTableID[8];
	UINT32 OEMRevision;
	UINT32 CreatorID;
	UINT32 CreatorRevision;
} ACPI_SDT_HEADER;

typedef struct __attribute__((__packed__)) ACPI_BGRT
{
	ACPI_SDT_HEADER Header;
	UINT16 Version;
	UINT8 Status;
	UINT8 ImageType;
	UINT64 ImageAddress;
	UINT32 ImageOffsetX;
	UINT32 ImageOffsetY;
} ACPI_BGRT;

typedef struct __attribute__((__packed__)) BMP_HEADER
{
	CHAR8 Signature[2];
	UINT32 Size;
	CHAR8 Reserved[4];
	UINT32 ImageDataOffset;
} BMP_HEADER;

static BOOLEAN Cleared = FALSE;

static BOOLEAN parse_acpi_sdt_entry(UINT64 address)
{
	ACPI_SDT_HEADER * acpi_sdt_header = (ACPI_SDT_HEADER *)address;

	if (CompareMem(acpi_sdt_header->Signature, "BGRT", 4)) return FALSE;
	PrintDebug(L"Found BGRT address: 0x%X%X\n\n", address>>32, address);

	ACPI_BGRT * acpi_bgrt = (ACPI_BGRT *)address;
	if (acpi_bgrt->Header.Length != sizeof(ACPI_BGRT) ||
		(acpi_bgrt->Header.Revision != 1 && acpi_bgrt->Header.Revision != 2) ||
		acpi_bgrt->Version != 1 || acpi_bgrt->ImageType != 0 || !acpi_bgrt->ImageAddress)
	{
		Print(L"Invalid or unsupported BGRT or boot image type\n");
		PrintDebug(L"BGRT.Length = %d [should be %d]\n", acpi_bgrt->Header.Length, sizeof(ACPI_BGRT));
		PrintDebug(L"BGRT.Revision = %d [should be 1 or 2]\n", acpi_bgrt->Header.Revision);
		PrintDebug(L"BGRT.Version = %d [should be 1]\n", acpi_bgrt->Version);
		PrintDebug(L"BGRT.ImageType = %d [should be 1 (bitmap)]\n", acpi_bgrt->ImageType);
		PrintDebug(L"BGRT.ImageAddress = %d [should not be zero]\n", acpi_bgrt->ImageAddress);

		return FALSE;
	}

	PrintDebug(L"Found boot logo bitmap address: 0x%X%X\n",
		acpi_bgrt->ImageAddress>>32, acpi_bgrt->ImageAddress);

	BMP_HEADER * bmp_header = (BMP_HEADER *)acpi_bgrt->ImageAddress;
	if (CompareMem(bmp_header->Signature, "BM", 2))
	{
		Print(L"Invalid bitmap header signature: %.2a\n", bmp_header->Signature);
		PrintDebug(L"BMP_Header.Signature = %.2a [should be BM]\n", bmp_header->Signature);
		return FALSE;
	}

	PrintDebug(L"Clearing bitmap data (Size: %d bytes)\n", bmp_header->Size);
	ZeroMem(bmp_header, bmp_header->Size);
	Cleared = TRUE;
	PrintDebug(L"Boot logo cleared!\n\n");

	return TRUE;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	InitializeLib(ImageHandle, SystemTable);
	PrintDebug(L"-- WinClear --\nClear the boot logo and start Windows\n\n");

	EFI_CONFIGURATION_TABLE *ConfigurationTable = ST->ConfigurationTable;
	UINTN ConfigurationTableSz = ST->NumberOfTableEntries;
	PrintDebug(L"%d entries in ConfigurationTable\n", ConfigurationTableSz);

	for (UINTN i = 0; i < ConfigurationTableSz; i++)
	{
		EFI_GUID efi_acpi_table_guid = {0x8868e871,0xe4f1,0x11d3,
			{0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}};

		if (CompareGuid(&efi_acpi_table_guid, &ConfigurationTable[i].VendorGuid) == 0)
		{
			PrintDebug(L"Found ACPI table address: 0x%X\n\n", ConfigurationTable[i].VendorTable);

			ACPI_RSDP * acpi_rsdp = (ACPI_RSDP *)ConfigurationTable[i].VendorTable;

			if (acpi_rsdp->Revision == 1)
			{
				UINT64 RsdtAddress = acpi_rsdp->RsdtAddress;
				UINT64 RsdtEntriesAddress = RsdtAddress + 36;
				ACPI_SDT_HEADER * acpi_rsdt_header = (ACPI_SDT_HEADER *)RsdtAddress;
				UINT32 * acpi_rsdt_entries = (UINT32 *)RsdtEntriesAddress;
				UINTN acpi_rsdt_number_of_entries = (acpi_rsdt_header->Length - 36) / sizeof(UINT32);
				PrintDebug(L"%d entries in RSDT\n", acpi_rsdt_number_of_entries);
				for (UINTN j = 0; j < acpi_rsdt_number_of_entries; j++)
				{
					if (parse_acpi_sdt_entry((UINT64)acpi_rsdt_entries[j])) break;
				}
			}
			else if (acpi_rsdp->Revision == 2)
			{
				ACPI_SDT_HEADER * acpi_xsdt_header = (ACPI_SDT_HEADER *)acpi_rsdp->XsdtAddress;
				UINT64 * acpi_xsdt_entries = (UINT64 *)(acpi_rsdp->XsdtAddress + 36);
				UINTN acpi_xsdt_number_of_entries = (acpi_xsdt_header->Length - 36) / sizeof(UINT64);
				PrintDebug(L"%d entries in XSDT\n", acpi_xsdt_number_of_entries);
				for (UINTN j = 0; j < acpi_xsdt_number_of_entries; j++)
				{
					if (parse_acpi_sdt_entry((UINT64)acpi_xsdt_entries[j])) break;
				}
			}
			else
			{
				Print(L"ACPI RSDP table revision unsupported. Cannot proceed\n\n");
			}

			break;
		}
	}

	if (!Cleared)
	{
		Print(L"Failed to clear boot logo\nStarting Windows Boot Manager in 3 seconds\n");
		uefi_call_wrapper(BS->Stall, 1, 3*1000*1000);
	}

	EFI_STATUS Ret;
	EFI_LOADED_IMAGE *LoadedImage;
	EFI_DEVICE_PATH * Path;
	EFI_FILE_HANDLE Root;
	EFI_FILE_HANDLE WindowsImageFile;
	EFI_HANDLE WindowsImage;

	Ret = uefi_call_wrapper(BS->OpenProtocol, 6, ImageHandle, &LoadedImageProtocol,
		(VOID **)&LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	ExitOnError(EFI_ERROR(Ret), (L"Error opening LoadedImageProtocol: %r\n"), Ret);

	Root = LibOpenRoot(LoadedImage->DeviceHandle);
	ExitOnError(!Root, L"Error opening ESP root directory: %r\n", EFI_LOAD_ERROR);
	Ret = uefi_call_wrapper(Root->Open, 5, Root, &WindowsImageFile,
		L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi", EFI_FILE_MODE_READ, 0);
	ExitOnError(EFI_ERROR(Ret), L"Could not open ESP:\\EFI\\Microsoft\\Boot\\bootmgfw.efi: %r\n", Ret);
	uefi_call_wrapper(WindowsImageFile->Close, 1, WindowsImageFile);
	uefi_call_wrapper(Root->Close, 1, Root);

	Path = FileDevicePath(LoadedImage->DeviceHandle, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi");
	ExitOnError(!Path, L"Error getting device path: %r\n", EFI_INVALID_PARAMETER);
	uefi_call_wrapper(BS->CloseProtocol, 4, ImageHandle, &LoadedImageProtocol, ImageHandle, NULL);

	Ret = uefi_call_wrapper(BS->LoadImage, 6, FALSE, ImageHandle, Path, NULL, 0, &WindowsImage);
	ExitOnError(EFI_ERROR(Ret), L"Error loading Windows Boot Manager image file: %r\n", Ret);
	Ret = uefi_call_wrapper(BS->StartImage, 3, WindowsImage, NULL, NULL);
	ExitOnError(EFI_ERROR(Ret), L"Error starting Windows Boot Manager image: %r\n", Ret);

	uefi_call_wrapper(BS->UnloadImage, 1, WindowsImage);
	FreePool(Path);
	return Ret;
}
