/*
 *  Fuck SecureBoot
 *  Copyright (C) 2020  a1ive
 *  Copyright (C) 2020 Samuel Tulach (@SamuelTulach)
 *  Copyright (C) 2019 Matthijs Lavrijsen (@Mattiwatti)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <efi.h>
#include <efilib.h>

#include <getvar.h>

/* https://github.com/SamuelTulach/efi-memory */

typedef struct _DummyProtocalData
{
    UINTN blank;
} DummyProtocalData;

static EFI_GUID ProtocolGuid =
{
    0x4655434b, 0x2053, 0x4543,
    { 0x55, 0x52, 0x45, 0x20, 0x42, 0x4f, 0x4f, 0x54 }
};

// VirtualAddressMap GUID (gEfiEventVirtualAddressChangeGuid)
static EFI_GUID VirtualGuid =
{
    0x13FA7698, 0xC831, 0x49C7,
    { 0x87, 0xEA, 0x8F, 0x43, 0xFC, 0xC2, 0x51, 0x96 }
};

// ExitBootServices GUID (gEfiEventExitBootServicesGuid)
static EFI_GUID ExitGuid =
{
    0x27ABF055, 0xB1B8, 0x4C26,
    { 0x80, 0x48, 0x74, 0x8F, 0x37, 0xBA, 0xA2, 0xDF }
};

static EFI_EVENT NotifyEvent = NULL;
static EFI_EVENT ExitEvent = NULL;
static EFI_GET_VARIABLE orig_get_variable = NULL;

static inline int
rt_strcmp (const CHAR16 *s1, const CHAR16 *s2)
{
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }
    return (int) (UINT16) *s1 - (int) (UINT16) *s2;
}

static EFI_STATUS EFIAPI
get_variable_wrapper (CHAR16 *VariableName, EFI_GUID *VendorGuid,
                      UINT32 *Attributes, UINTN *DataSize, VOID *Data)
{
    EFI_STATUS status;
    CHAR16 sb[] = L"SecureBoot";

    status = orig_get_variable (VariableName, VendorGuid,
                                Attributes, DataSize, Data);
    if (rt_strcmp (sb, VariableName) == 0)
    {
        *(UINT8 *) Data = 0;
        *DataSize = 1;
    }
    return status;
}

static VOID EFIAPI
set_virtual_address_map_event (EFI_EVENT Event, VOID* Context)
{
    RT->ConvertPointer(0, (VOID **)&orig_get_variable);
    RtLibEnableVirtualMappings();
    NotifyEvent = NULL;
}

static VOID EFIAPI
exit_bs_event (EFI_EVENT Event, VOID* Context)
{
    // This event is called only once so close it
    BS->CloseEvent(ExitEvent);
    ExitEvent = NULL;
    // Boot services are now not avaible
    BS = NULL;

    Print(L"Exit BootServices ...\n");
}

// Replaces service table pointer with desired one
// returns original
static VOID *
set_service_pointer (EFI_TABLE_HEADER *ServiceTableHeader,
                     VOID **ServiceTableFunction, VOID *NewFunction)
{
    // We don't want to fuck up the system
    if (ServiceTableFunction == NULL || NewFunction == NULL)
        return NULL;

    // Make sure boot services pointers are not null
    ASSERT (BS != NULL);
    ASSERT (BS->CalculateCrc32 != NULL);

    // Raise task priority level
    CONST EFI_TPL Tpl = BS->RaiseTPL (TPL_HIGH_LEVEL);

    // Swap the pointers
    // GNU-EFI and InterlockedCompareExchangePointer are not friends
    VOID* OriginalFunction = *ServiceTableFunction;
    *ServiceTableFunction = NewFunction;

    // Change the table CRC32 signature
    ServiceTableHeader->CRC32 = 0;
    BS->CalculateCrc32 ((UINT8*)ServiceTableHeader,
                        ServiceTableHeader->HeaderSize, &ServiceTableHeader->CRC32);

    // Restore task priority level
    BS->RestoreTPL (Tpl);

    return OriginalFunction;
}

static EFI_STATUS EFI_FUNCTION
efi_unload (EFI_HANDLE image_handle)
{
    // We don't want our driver to be unloaded
    return EFI_ACCESS_DENIED;
}

EFI_STATUS
hook_get_variable (EFI_HANDLE image_handle)
{
    EFI_LOADED_IMAGE *LoadedImage = NULL;
    EFI_STATUS status;

    status = BS->OpenProtocol (image_handle, &LoadedImageProtocol,
                               (VOID**)&LoadedImage, image_handle,
                               NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (status != EFI_SUCCESS)
    {
        Print (L"Can't open LoadedImageProtocol\n");
        return status;
    }
    // Install our protocol interface
    // This is needed to keep our driver loaded
    DummyProtocalData dummy = { 0 };
    status = LibInstallProtocolInterfaces (&image_handle, &ProtocolGuid,
                                           &dummy, NULL);
    // Return if interface failed to register
    if (status != EFI_SUCCESS)
    {
        Print (L"Can't register interface\n");
        return status;
    }
    // Set our image unload routine
    LoadedImage->Unload = (EFI_IMAGE_UNLOAD) efi_unload;
    // Create global event for VirtualAddressMap
    status = BS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_NOTIFY,
                                set_virtual_address_map_event,
                                NULL, &VirtualGuid, &NotifyEvent);
    // Return if event create failed
    if (status != EFI_SUCCESS)
    {
        Print(L"Can't create SetVirtualAddressMapEvent\n");
        return status;
    }
    // Create global event for ExitBootServices
    status = BS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_NOTIFY,
                                exit_bs_event,
                                NULL, &ExitGuid, &ExitEvent);
    // Return if event create failed (yet again)
    if (status != EFI_SUCCESS)
    {
        Print (L"Can't create ExitBootServicesEvent\n");
        return status;
    }

    orig_get_variable =
        (EFI_GET_VARIABLE) set_service_pointer (&RT->Hdr, (VOID**)&RT->GetVariable,
                                                (VOID**)&get_variable_wrapper);
    return EFI_SUCCESS;
}
