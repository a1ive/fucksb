/*
 *  Fuck SecureBoot
 *  Copyright (C) 2020  a1ive
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

#include <sbpolicy.h>
#include <getvar.h>

#define FUCKSB_TITLE \
    L"\n" \
    L"\n  _____ _     ____  _  __ ____  ____  " \
    L"\n /    // \\ /\\/   _\\/ |/ // ___\\/  __\\ " \
    L"\n |  __\\| | |||  /  |   / |    \\| | // " \
    L"\n | |   | \\_/||  \\__|   \\ \\___ || |_\\\\ " \
    L"\n \\_/   \\____/\\____/\\_|\\_\\\\____/\\____/ " \
    L"\n       Copyright (c) 2020 a1ive       " \
    L"\n\n"

#define FLAG_FORCE_INSTALL  0x01
#define FLAG_INSTALL_POLICY 0x02
#define FLAG_ENABLE_SB      0x04
#define FLAG_HOOK_BS_GETVAR 0x08
#define FLAG_NO_STALL       0x10
#define FLAG_RESERVED_1     0x20
#define FLAG_RESERVED_2     0x40
#define FLAG_RESERVED_3     0x80

static UINT8 fucksb_flag;

static BOOLEAN
check_flag (UINT8 flag)
{
    return (fucksb_flag & flag) ? TRUE : FALSE;
}

static void
print_version (void)
{
  UINT16 major, minor;
  UINT8 minor_1, minor_2;
  major = ST->Hdr.Revision >> 16;
  minor = ST->Hdr.Revision & 0xffff;
  minor_1 = minor / 10;
  minor_2 = minor % 10;
  Print (L"UEFI v%d.%d", major, minor_1);
  if (minor_2)
    Print (L".%d", minor_2);
  Print (L" (%s, 0x%X)\n", ST->FirmwareVendor, ST->FirmwareRevision);
}

static void
get_flag (void)
{
    UINT8 i;
    UINTN size = sizeof (fucksb_flag);
    EFI_STATUS status;
    EFI_GUID gv_guid = EFI_GLOBAL_VARIABLE;

    status = RT->GetVariable (L"FuckSBFlag", &gv_guid, NULL, &size, &fucksb_flag);
    if (status != EFI_SUCCESS)
        fucksb_flag = 0;

    Print (L"Flags: ");
    for (i = 0; i < 8; i++)
    {
        if (check_flag (0x01 << i))
            Print (L"(*)");
        else
            Print (L"( )");
    }
    Print (L"\n");
}

EFI_STATUS
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status = EFI_SUCCESS;

    InitializeLib (image_handle, systab);

    CONST UINTN orig_attr = ST->ConOut->Mode->Attribute;
    CONST UINTN bg = ((orig_attr >> 4) & 0x7);
    ST->ConOut->SetAttribute (ST->ConOut, EFI_GREEN | bg);
    ST->ConOut->ClearScreen (ST->ConOut);
    Print(FUCKSB_TITLE);
    print_version ();

    /* Print flags */
    get_flag ();

    if (! check_secureboot () && ! check_flag (FLAG_FORCE_INSTALL))
        goto fail;

    if (check_flag (FLAG_INSTALL_POLICY))
    {
        status = security_policy_install ();
        if (status != EFI_SUCCESS)
            Print (L"Failed to install override security policy\n");
    }

    if (check_flag (FLAG_HOOK_BS_GETVAR))
        status = bs_hook_get_variable (check_flag (FLAG_ENABLE_SB));
    else
        status = hook_get_variable (image_handle, check_flag (FLAG_ENABLE_SB));
    if (status != EFI_SUCCESS)
        Print (L"Failed to hook GetVariable");

fail:
    ST->ConOut->SetAttribute (ST->ConOut, orig_attr);
    if (!check_flag (FLAG_NO_STALL))
      BS->Stall (1000000);
    return status;
}
