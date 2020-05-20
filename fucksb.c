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

EFI_STATUS
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status;

    InitializeLib (image_handle, systab);

    if (! check_secureboot ())
        return EFI_SUCCESS;

    status = security_policy_install ();
    if (status != EFI_SUCCESS)
    {
        Print (L"Failed to install override security policy\n");
        return status;
    }
    hook_get_variable (image_handle);
    if (status != EFI_SUCCESS)
    {
        Print (L"Failed to hook GetVariable");
        return status;
    }
    return EFI_SUCCESS;
}
