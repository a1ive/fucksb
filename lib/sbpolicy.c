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

static EFI_GUID SECURITY_PROTOCOL_GUID =
{
    0xA46423E3, 0x4617, 0x49f1,
    { 0xB9, 0xFF, 0xD1, 0xBF, 0xA9, 0x11, 0x58, 0x39 }
};
static EFI_GUID SECURITY2_PROTOCOL_GUID =
{
    0x94ab2f58, 0x1438, 0x4ef1,
    { 0x91, 0x52, 0x18, 0x94, 0x1a, 0x3a, 0x0e, 0x68 }
};

struct _EFI_SECURITY2_PROTOCOL;
struct _EFI_SECURITY_PROTOCOL;
typedef struct _EFI_SECURITY2_PROTOCOL EFI_SECURITY2_PROTOCOL;
typedef struct _EFI_SECURITY_PROTOCOL EFI_SECURITY_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_SECURITY_FILE_AUTHENTICATION_STATE)
           (const EFI_SECURITY_PROTOCOL *This,
            UINT32 AuthenticationStatus,
            const EFI_DEVICE_PATH_PROTOCOL *File);
typedef EFI_STATUS (EFIAPI *EFI_SECURITY2_FILE_AUTHENTICATION)
           (const EFI_SECURITY2_PROTOCOL *This,
            const EFI_DEVICE_PATH_PROTOCOL *DevicePath,
            VOID *FileBuffer,
            UINTN FileSize,
            BOOLEAN BootPolicy);

struct _EFI_SECURITY2_PROTOCOL
{
    EFI_SECURITY2_FILE_AUTHENTICATION FileAuthentication;
};

struct _EFI_SECURITY_PROTOCOL
{
    EFI_SECURITY_FILE_AUTHENTICATION_STATE  FileAuthenticationState;
};

static EFI_STATUS EFIAPI
security2_policy_authentication (const EFI_SECURITY2_PROTOCOL *This,
                                 const EFI_DEVICE_PATH_PROTOCOL *DevicePath,
                                 VOID *FileBuffer, UINTN FileSize,
                                 BOOLEAN BootPolicy)
{
    return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI
security_policy_authentication (const EFI_SECURITY_PROTOCOL *This,
                                UINT32 AuthenticationStatus,
                                const EFI_DEVICE_PATH_PROTOCOL *DevicePathConst)
{
    return EFI_SUCCESS;
}

static EFIAPI EFI_SECURITY_FILE_AUTHENTICATION_STATE esfas = NULL;
static EFIAPI EFI_SECURITY2_FILE_AUTHENTICATION es2fa = NULL;

EFI_STATUS
security_policy_install (void)
{
    EFI_SECURITY_PROTOCOL *security_protocol = NULL;
    EFI_SECURITY2_PROTOCOL *security2_protocol = NULL;
    EFI_STATUS status;

    if (esfas)
        /* Already Installed */
        return EFI_ALREADY_STARTED;

    /* Don't bother with status here.  The call is allowed
     * to fail, since SECURITY2 was introduced in PI 1.2.1
     * If it fails, use security2_protocol == NULL as indicator */
    Print (L"Locate Security2Protocol ... ");
    status = BS->LocateProtocol (&SECURITY2_PROTOCOL_GUID, NULL,
                                 (VOID **)&security2_protocol);
    if (status != EFI_SUCCESS)
        Print (L"NOT FOUND\n");
    else
        Print (L"OK\n");

    Print (L"Locate SecurityProtocol ... ");
    status = BS->LocateProtocol (&SECURITY_PROTOCOL_GUID, NULL,
                                 (VOID **)&security_protocol);
    if (status != EFI_SUCCESS)
    {
        Print (L"FAIL\nFailed to locate SecurityProtocol\n");
        return status;
    }
    Print (L"OK\n");

    if (security2_protocol)
    {
        Print (L"Hook Security2Protocol->FileAuthentication ... ");
        es2fa = security2_protocol->FileAuthentication;
        security2_protocol->FileAuthentication = security2_policy_authentication;
        /* check for security policy in write protected memory */
        if (security2_protocol->FileAuthentication
            !=  security2_policy_authentication)
        {
            Print (L"ACCESS DENIED\n");
            return EFI_ACCESS_DENIED;
        }
        Print (L"OK\n");
    }

    Print (L"Hook SecurityProtocol->FileAuthenticationState ... ");
    esfas = security_protocol->FileAuthenticationState;
    security_protocol->FileAuthenticationState = security_policy_authentication;
    /* check for security policy in write protected memory */
    if (security_protocol->FileAuthenticationState
        !=  security_policy_authentication)
    {
        Print (L"ACCESS DENIED\n");
        return EFI_ACCESS_DENIED;
    }
    Print (L"OK\n");

    return EFI_SUCCESS;
}

BOOLEAN
check_secureboot (void)
{
    EFI_STATUS status;
    EFI_GUID gv_guid = EFI_GLOBAL_VARIABLE;
    UINT8 sb;
    UINTN datasize = sizeof (sb);

    Print (L"Check secure boot status ... ");
    status = RT->GetVariable (L"SecureBoot", &gv_guid, NULL, &datasize, &sb);
    if (status != EFI_SUCCESS)
    {
        Print (L"UNSUPPORTED\n");
        return FALSE;
    }
    if (!sb)
    {
        Print (L"DISABLED\n");
        return FALSE;
    }
    Print (L"ENABLED\n");
    return TRUE;
}
