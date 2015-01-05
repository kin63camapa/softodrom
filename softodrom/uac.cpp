#include <windows.h>
BOOL (*CheckTokenMembership)(HANDLE,PSID,PBOOL);

int IsRunAsAdmin()
{
    int ret = -1;
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = NULL;
    HINSTANCE dllHandle = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &pAdministratorsGroup))
    {
        dllHandle = LoadLibrary(TEXT("Advapi32.dll"));
        if (NULL != dllHandle)
        {
            CheckTokenMembership = (BOOL(*)(HANDLE,PSID,PBOOL))GetProcAddress(dllHandle,"CheckTokenMembership");
            if (NULL != CheckTokenMembership)
            {
                CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin);
                ret = fIsRunAsAdmin;
            }
            FreeLibrary(dllHandle);
        }
        else
        {
            ret = -2;
            GetLastError();
        }
    }
    else
    {
        ret = -3;
        GetLastError();
    }
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }
    return ret;
}


//
//   FUNCTION: IsProcessElevated()
//
//   PURPOSE: The function gets the elevation information of the current
//   process. It dictates whether the process is elevated or not. Token
//   elevation is only available on Windows Vista and newer operating
//   systems, thus IsProcessElevated throws a C++ exception if it is called
//   on systems prior to Windows Vista. It is not appropriate to use this
//   function to determine whether a process is run as administartor.
//
//   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if
//   it is not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception
//   which contains the Win32 error code of the failure. For example, if
//   IsProcessElevated is called on systems prior to Windows Vista, the error
//   code will be ERROR_INVALID_PARAMETER.
//
//   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the
//   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
//   TokenElevationTypeFull) of the process. It is different from
//   TokenElevation in that, when UAC is turned off, elevation type always
//   returns TokenElevationTypeDefault even though the process is elevated
//   (Integrity Level == High). In other words, it is not safe to say if the
//   process is elevated based on elevation type. Instead, we should use
//   TokenElevation.
//
//   EXAMPLE CALL:
//     try
//     {
//         if (IsProcessElevated())
//             wprintf (L"Process is elevated\n");
//         else
//             wprintf (L"Process is not elevated\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
//     }
//
//BOOL IsProcessElevated()
//{
//    BOOL fIsElevated = FALSE;
//    DWORD dwError = ERROR_SUCCESS;
//    HANDLE hToken = NULL;

//    // Open the primary access token of the process with TOKEN_QUERY.
//    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
//    {
//        dwError = GetLastError();
//        goto Cleanup;
//    }

//    // Retrieve token elevation information.
//    TOKEN_ELEVATION elevation;
//    DWORD dwSize;
//    if (!GetTokenInformation(hToken, TokenElevation, &elevation,
//        sizeof(elevation), &dwSize))
//    {
//        // When the process is run on operating systems prior to Windows
//        // Vista, GetTokenInformation returns FALSE with the
//        // ERROR_INVALID_PARAMETER error code because TokenElevation is
//        // not supported on those operating systems.
//        dwError = GetLastError();
//        goto Cleanup;
//    }

//    fIsElevated = elevation.TokenIsElevated;

//Cleanup:
//    // Centralized cleanup for all allocated resources.
//    if (hToken)
//    {
//        CloseHandle(hToken);
//        hToken = NULL;
//    }

//    // Throw the error if something failed in the function.
//    if (ERROR_SUCCESS != dwError)
//    {
//        throw dwError;
//    }

//    return fIsElevated;
//}
