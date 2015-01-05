
//// #define _WIN32_WINNT 0x0502 // Windows Server 2003 family

//// For Win Xp, change accordingly...

//// #define _WIN32_WINNT 0x0501

//// #define _WIN32_WINNT 0x0500 // Windows 2000

//// #define _WIN32_WINNT 0x0400 // Windows NT 4.0

//// #define _WIN32_WINDOWS 0x0500 // Windows ME

//// #define _WIN32_WINDOWS 0x0410 // Windows 98

//// #define _WIN32_WINDOWS 0x0400 // Windows 95



#include <windows.h>

//#include <stdio.h>

//#define BUFSIZE 80



int func(int argc, char *argv[])

{



//    BOOL bOsVersionInfoEx;

//    HKEY hKey;

//    LONG lRet;



//    // Try calling GetVersionEx() using the OSVERSIONINFOEX structure.

//    // If that fails, try using the OSVERSIONINFO structure.
/*    OSVERSIONINFOEX osver;
    ZeroMemory(&osver, sizeof(OSVERSIONINFOEX));
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if(!(GetVersionEx((OSVERSIONINFO *) &osver)))
    {
        osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if(!GetVersionEx((OSVERSIONINFO *) &osver))
            return FALSE;
        else
            printf("Buffer for the structure size is OK.\n");
    }
    else
        printf("GetVersionEx() is OK.\n")*/;

}

//    switch (osver.dwPlatformId)

//    {

//    //Test for the Windows NT product family.

//    case VER_PLATFORM_WIN32_NT:

//        // Test for the specific product family.

//        if(osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2)

//            printf("Microsoft Windows Server 2003 family, ");

//        if(osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1)

//            printf("Microsoft Windows XP ");

//        if(osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0)

//            printf("Microsoft Windows 2000 ");

//        if(osver.dwMajorVersion <4> BUFSIZE)

//            return FALSE;

//        else

//            printf("RegQueryValueEx() is OK.\n");

//        RegCloseKey(hKey);

//        if(lstrcmpi(L"WINNT", szProductType) == 0)

//            printf("Workstation ");

//        if(lstrcmpi(L"LANMANNT", szProductType) == 0)

//            printf("Server ");

//        if(lstrcmpi(L"SERVERNT", szProductType) == 0)

//            printf("Advanced Server ");

//        printf("%d.%d ", osver.dwMajorVersion, osver.dwMinorVersion);

//    }



//    // Display service pack (if any) and build number.

//    if(osver.dwMajorVersion == 4 && lstrcmpi(osver.szCSDVersion, L"Service Pack 6") == 0)

//    {

//        // Test for SP6 versus SP6a.

//        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\WindowsNT\\CurrentVersion\\Hotfix\\Q246009", 0, KEY_QUERY_VALUE, &hKey);

//        if(lRet == ERROR_SUCCESS)

//        {

//            printf("RegOpenKeyEx() is OK.\n");

//            printf("Service Pack 6a (Build %d)\n", osver.dwBuildNumber & 0xFFFF);

//        }

//        // Windows NT 4.0 prior to SP6a

//        else

//        {

//            printf("%S (Build %d)\n", osver.szCSDVersion, osver.dwBuildNumber & 0xFFFF);

//        }

//        RegCloseKey(hKey);

//    }

//    // Windows NT 3.51 and earlier or Windows 2000 and later

//    else

//    {

//        printf("%S (Build %d)\n", osver.szCSDVersion, osver.dwBuildNumber & 0xFFFF);

//    }

//    break;



//    // Test for the Windows 95 product family.

//    case VER_PLATFORM_WIN32_WINDOWS:

//        if(osver.dwMajorVersion == 4 && osver.dwMinorVersion == 0)

//        {

//            printf("Microsoft Windows 95 ");

//            if(osver.szCSDVersion[1] == 'C' || osver.szCSDVersion[1] == 'B')

//                printf("OSR2 ");

//        }

//        if(osver.dwMajorVersion == 4 && osver.dwMinorVersion == 10)

//        {

//            printf("Microsoft Windows 98 ");

//            if(osver.szCSDVersion[1] == 'A')

//                printf("SE ");

//        }

//        if((osver.dwMajorVersion == 4) && (osver.dwMinorVersion == 90))

//        {

//            printf("Microsoft Windows Millennium Edition\n");

//        }

//        break;

//    case VER_PLATFORM_WIN32s:

//        printf("Microsoft Win32s\n");

//        break;

//}

//return TRUE;

//}
