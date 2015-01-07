#include <windows.h>
#include <iostream>
#include "softodromglobal.h"
#include "errcodes.h"

QSettings *AppSettings;
OSINFO OSinfo;

void hideConsole()
{
    FreeConsole();
}

QString SDtranslateKit(QString kit)
{
    kit.replace("nogroup",QString::fromUtf8("Вне наборов"));
    kit.replace("base",QString::fromUtf8("Базовый"));
    kit.replace("browsers",QString::fromUtf8("Браузеры"));
    kit.replace("runtimes",QString::fromUtf8("Среды исполнения"));
    kit.replace("home",QString::fromUtf8("Для дома"));
    kit.replace("kit",QString::fromUtf8("Поднаборы"));
    kit.replace("antiviruses",QString::fromUtf8("Антивирусы"));
    kit.replace("nogroup",QString::fromUtf8("Вне наборов"));
    kit.replace("nogroup",QString::fromUtf8("Вне наборов"));
    return kit;
}

void setDefaultConfig(bool replace)
{
    if (replace) AppSettings->clear();
    if (!AppSettings->contains("SoftStartupFolder"))
        AppSettings->setValue("SoftStartupFolder",QString::fromUtf8("..\\"));
    if (!AppSettings->contains("UpdateStartupFolder"))
        AppSettings->setValue("UpdateStartupFolder",QString::fromUtf8("..\\updates\\"));
    if (!AppSettings->contains("RecursiveSearch"))
        AppSettings->setValue("RecursiveSearch",bool(TRUE));
    if (!AppSettings->contains("UseAuto.bat"))
        AppSettings->setValue("UseAuto.bat",bool(TRUE));
    if (!AppSettings->contains("UseAuto.cmd"))
        AppSettings->setValue("UseAuto.cmd",bool(FALSE));
    if (!AppSettings->contains("UseInfo.txt"))
        AppSettings->setValue("UseInfo.txt",bool(TRUE));
    if (!AppSettings->contains("UseIconsFromExe"))
        AppSettings->setValue("UseIconsFromExe",bool(TRUE));
    if (!AppSettings->contains("IgnoreDuplicate"))
        AppSettings->setValue("IgnoreDuplicate",bool(TRUE));
    if (!AppSettings->contains("Info.txtIsPrefer"))
        AppSettings->setValue("Info.txtIsPrefer",bool(TRUE));
    if (!AppSettings->contains("IngnoreBathOnly"))
        AppSettings->setValue("IngnoreBathOnly",bool(TRUE));
    if (!AppSettings->contains("SmartOsFolder"))
        AppSettings->setValue("SmartOsFolder",bool(TRUE));
    if (!AppSettings->contains("AddCurrentPath"))
        AppSettings->setValue("AddCurrentPath",bool(FALSE));
    if (!AppSettings->contains("DebugMode"))
        AppSettings->setValue("DebugMode",bool(FALSE));
    AppSettings->beginGroup("View");
    if (!AppSettings->contains("ShowInfoBlock"))
        AppSettings->setValue("ShowInfoBlock",bool(FALSE));
    if (!AppSettings->contains("ShowProgressBars"))
        AppSettings->setValue("ShowProgressBars",bool(TRUE));
    if (!AppSettings->contains("AppBoxWidth"))
        AppSettings->setValue("AppBoxWidth",int(290));
    if (!AppSettings->contains("AppBoxHeight"))
        AppSettings->setValue("AppBoxHeight",int(250));
    if (!AppSettings->contains("ShowConsole"))
        AppSettings->setValue("ShowConsole",bool(FALSE));
    AppSettings->endGroup();
}

void SDDebugMessage(QString header,QString text,bool ignoreDebug, SDDebugMessageIco ico)
{
    std::cerr << "\n" << QString(header.toLocal8Bit()).toStdString().c_str()
              << "\n" << QString(text.toLocal8Bit()).toStdString().c_str() << "\n";

    if (AppSettings->value("DebugMode").toBool() || ignoreDebug)
    {
        UINT icon = static_cast<UINT>(ico);
        MessageBox(
                    NULL,
                    text.toStdWString().c_str(),
                    header.toStdWString().c_str(),
                    icon
                    );
    }
}

HANDLE SDRunExternal(QString app, QString parameters, QString dir, bool RunWithElevation)
{
    SHELLEXECUTEINFO executeInfo;
    executeInfo.cbSize = sizeof(SHELLEXECUTEINFO);

    executeInfo.fMask = 0;
    executeInfo.hwnd = 0;
    if (RunWithElevation) executeInfo.lpVerb = L"runas";
    else
    {
        executeInfo.lpVerb = L"open";
        executeInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    }
    app.replace("/","\\");
    //app.replace("\\\\","\\\\?\\UNC\\");
    executeInfo.lpFile = app.toStdWString().c_str();
    executeInfo.lpParameters = parameters.isEmpty()
        ? 0 : parameters.toStdWString().c_str();
    executeInfo.lpDirectory = dir.isEmpty()
        ? 0 : dir.toStdWString().c_str();
    executeInfo.nShow = SW_NORMAL;
    if (ShellExecuteEx(&executeInfo))
        return executeInfo.hProcess;
    else
        return NULL;

}

QString ExpandEnvironmentString(QString str)
{
    TCHAR szEnvPath[MAX_PATH];
    ExpandEnvironmentStrings(str.toStdWString().c_str(), szEnvPath, MAX_PATH );
    return QString::fromWCharArray(szEnvPath);

}

QString GetVer(QString fileName)
{
    WCHAR strTmp[512];
    DWORD handle;         //didn't actually use (dummy var)
    int infoSize = (int) GetFileVersionInfoSize(fileName.toStdWString().c_str(), &handle);
    if(infoSize == 0) return "ERROR: Function <GetFileVersionInfoSize> unsuccessful!";
    LPVOID pBlock;
    pBlock = new BYTE[infoSize];
    int bResult = GetFileVersionInfo(fileName.toStdWString().c_str(), handle, infoSize, pBlock);
    if(bResult == 0)
    {
        return "ERROR: Function <GetFileVersionInfo> unsuccessful!";
    }
    // Structure used to store enumerated languages and code pages.
    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    }*lpTranslate;
    LPVOID lpBuffer;
    UINT dwBytes;
    int langNumber;
    // Read the list of languages and code pages.
    bResult = VerQueryValue(pBlock, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &dwBytes);
    if(bResult == 0)
    {
        return "ERROR: Function <VerQueryValue> for Translation unsuccessful!";
    }
    langNumber = dwBytes/sizeof(struct LANGANDCODEPAGE);
    //langNumber always must be equal 1
    if(langNumber != 1)
    {
        return QString("ERROR: Languages number: %1.").arg(langNumber);
    }
    HRESULT hr;
    for(int i=0; i<langNumber; i++)
    {
        hr = wsprintfW(strTmp, TEXT("\\StringFileInfo\\%04X%04X\\FileVersion"),lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
        if (FAILED(hr))
        {
            return "ERROR: Unable to get information in this language!";
        }
        // Retrieve file description for language and code page "i".

        if (VerQueryValue(pBlock, L"\\", &lpBuffer, &dwBytes))
        {
            VS_FIXEDFILEINFO *fi = (VS_FIXEDFILEINFO*)lpBuffer;
            fileName.clear();//теперь это версия %)
            QTextStream(&fileName) << HIWORD(fi->dwFileVersionMS) << "." << LOWORD(fi->dwFileVersionMS) << "." << HIWORD(fi->dwFileVersionLS) << "." << LOWORD(fi->dwFileVersionLS);
        }else
        {
            return "ERROR: Function <VerQueryValue> for StringFileInfo unsuccessful!";
        }
    }
    return fileName;
}

bool OSINFO::isRunAsAdmin()
{

    BOOL (*CheckTokenMembership)(HANDLE,PSID,PBOOL);

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
            SDDebugMessage("OSINFO::isRunAsAdmin()","LoadLibrary(\"Advapi32.dll\") failed: "+code2text(GetLastError()));
        }
    }
    else
    {
        ret = -3;
        SDDebugMessage("OSINFO::isRunAsAdmin()","AllocateAndInitializeSid() failed: "+code2text(GetLastError()));
    }
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }
    return ret;
}

bool OSINFO::isWow64()
{
#ifdef WIN64
    return is64build = true;
#else
    is64build = false;
    BOOL bIsWow64 = FALSE;
    typedef BOOL (APIENTRY *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    HMODULE module = GetModuleHandle(TEXT("kernel32.dll"));
    if(NULL == module)
    {
        error("Can not GetModuleHandle from kernel32.dll\nProbably, system is X86!\nCoder id Invalid!");
        return 0;
    }
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(module, "IsWow64Process");
    if(fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            error("API fnIsWow64Process() is FAIL!\nProbably, system is X86!\nCoder id Invalid!");
            return FALSE;
        }
    }
    else
    {
        error("Can not find API fnIsWow64Process() in kernel32.dll\nProbably, system is X86!\nCoder id Invalid!");
        return FALSE;
    }
    return bIsWow64 != FALSE;
#endif
}

QString OSINFO::toString(QString format)
{
    if (format.isEmpty())
        return QString("%1.%2").arg(MajorVersion).arg(MinorVersion);
    else
    {
        format = format.replace("%MajorVersion%",QString::number(MajorVersion));
        format = format.replace("%MinorVersion%",QString::number(MinorVersion));
        format = format.replace("%BuildNumber%",QString::number(BuildNumber));
        format = format.replace("%szCSDVersion%",szCSDVersion);
        format = format.replace("%ServicePackMajor%",QString::number(ServicePackMajor));
        format = format.replace("%ServicePackMinor%",QString::number(ServicePackMinor));
        format = format.replace("%SuiteMaskN%",QString::number(SuiteMask));
        switch (SuiteMask)
        {
        case VER_SUITE_BACKOFFICE:
            format = format.replace("%SuiteMask%","BackOffice");
            break;
        case VER_SUITE_BLADE:
            format = format.replace("%SuiteMask%","Web Edition");
            break;
        case VER_SUITE_COMPUTE_SERVER:
            format = format.replace("%SuiteMask%","Compute Cluster Edition");
            break;
        case VER_SUITE_DATACENTER:
            if (WinNoarch == W2003) format = format.replace("%SuiteMask%","Datacenter Edition");
            if (WinNoarch == W2KSERVER) format = format.replace("%SuiteMask%","Datacenter Server");
            format = format.replace("%SuiteMask%","Datacenter");
            break;
        case VER_SUITE_ENTERPRISE:
            if (WinNoarch == W2003) format = format.replace("%SuiteMask%","Enterprise Edition");
            if (WinNoarch == W2KSERVER) format = format.replace("%SuiteMask%","Advanced Server");
            format = format.replace("%SuiteMask%","Enterprise");
            break;
        case VER_SUITE_SINGLEUSERTS:
            format = format.replace("%SuiteMask%","Remote Desktop one session limit");
            break;
        case VER_SUITE_SMALLBUSINESS:
            format = format.replace("%SuiteMask%","Small Business Server");
            break;
        case VER_SUITE_STORAGE_SERVER:
            format = format.replace("%SuiteMask%","Windows Storage Server");
            break;
        case VER_SUITE_TERMINAL:
            format = format.replace("%SuiteMask%","Terminal Services");
            break;
//        case VER_SUITE_WH_SERVER:
//            format = format.replace("%SuiteMask%","Home Server");
//            break;
        }
        format = format.replace("%ProductTypeN%",QString::number(ProductType));
        format = format.replace("%ProductTypeN%",QString(ProductType));
        format = format.replace("%ReservedN%",QString::number(Reserved));
        format = format.replace("%Reserved%",QString(Reserved));
        switch (WinNoarch)
        {
        case W95: format = format.replace("%Name%","95"); break;
        case W98: format = format.replace("%Name%","98"); break;
        case WME: format = format.replace("%Name%","ME"); break;
        case W2KSERVER: format = format.replace("%Name%","2000"); break;
        case W2000: format = format.replace("%Name%","2000"); break;
        case WXP: format = format.replace("%Name%","XP"); break;
        case W2003: format = format.replace("%Name%","2003"); break;
        case WVISTA: format = format.replace("%Name%","Vista"); break;
        case W2008: format = format.replace("%Name%","2008"); break;
        case W7: format = format.replace("%Name%","7"); break;
        case W2008R2: format = format.replace("%Name%","2008R2"); break;
        case W8: format = format.replace("%Name%","8"); break;
        case W2012: format = format.replace("%Name%","2012"); break;
        case W8_1: format = format.replace("%Name%","8.1"); break;
        case W2012R2: format = format.replace("%Name%","2012R2"); break;
        case W10: format = format.replace("%Name%","10"); break;
        case W2014: format = format.replace("%Name%","Technical Preview"); break;
        case WINVALID: format = format.replace("%Name%","Unknown"); break;
        default: format = format.replace("%Name%","Unknown"); WinNoarch = WINVALID; break;
        }
        if (is64)
        {
            format = format.replace("%arch%","x64");
            format = format.replace("%Arch%","X64");
            format = format.replace("%Bits%","64");
            format = format.replace("%Is64N%","1");
            format = format.replace("%IS64%","TRUE");
            format = format.replace("%Is64%","True");
            format = format.replace("%is64%","true");
            format = format.replace("%Is32N%","0");
            format = format.replace("%IS32%","FALSE");
            format = format.replace("%Is32%","False");
            format = format.replace("%is32%","false");
        }
        else
        {
            format = format.replace("%arch%","x86");
            format = format.replace("%Arch%","X86");
            format = format.replace("%Bits%","32");
            format = format.replace("%Is64N%","0");
            format = format.replace("%IS64%","FALSE");
            format = format.replace("%Is64%","False");
            format = format.replace("%is64%","false");
            format = format.replace("%Is32N%","1");
            format = format.replace("%IS32%","TRUE");
            format = format.replace("%Is32%","True");
            format = format.replace("%is32%","true");
        }
        if (isAdmin)
        {
            format = format.replace("%Privileges%","Admin");
            format = format.replace("%isAdminN%","1");
            format = format.replace("%isAdminN%","true");
            format = format.replace("%IsAdminN%","True");
            format = format.replace("%ISAdminN%","TRUE");
        }
        else
        {
            format = format.replace("%Privileges%","User");
            format = format.replace("%isAdminN%","0");
            format = format.replace("%isAdminN%","false");
            format = format.replace("%IsAdminN%","False");
            format = format.replace("%ISAdminN%","FALSE");
        }
        return format;
    }
}

QString OSINFO::error(QString er)
{
    if (er.size()) lastError = QString::fromUtf8(er.toAscii());
    return lastError;
}

bool OSINFO::init()
{
    typedef struct _RTL_OSVERSIONINFOW {
      ULONG  dwOSVersionInfoSize;
      ULONG  dwMajorVersion;
      ULONG  dwMinorVersion;
      ULONG  dwBuildNumber;
      ULONG  dwPlatformId;
      WCHAR  szCSDVersion[128];
    } *PRTL_OSVERSIONINFOW;

    struct _RTL_OSVERSIONINFOEXW {
      ULONG  dwOSVersionInfoSize;
      ULONG  dwMajorVersion;
      ULONG  dwMinorVersion;
      ULONG  dwBuildNumber;
      ULONG  dwPlatformId;
      WCHAR  szCSDVersion[128];
      USHORT  wServicePackMajor;
      USHORT  wServicePackMinor;
      USHORT  wSuiteMask;
      UCHAR  wProductType;
      UCHAR  wReserved;
    };

    BOOL (*RtlGetVersion)(PRTL_OSVERSIONINFOW);

    error("Пипец какая странная ошибка");
    MajorVersion = 0;
    isServer = 0;
    _RTL_OSVERSIONINFOEXW osver;
    ZeroMemory(&osver, sizeof(_RTL_OSVERSIONINFOEXW));
    osver.dwOSVersionInfoSize = sizeof(_RTL_OSVERSIONINFOEXW);
    HINSTANCE dllHandle;
    if ((dllHandle = LoadLibrary(TEXT("ntdll.dll"))))
    {
        if ((RtlGetVersion = (BOOL(*)(PRTL_OSVERSIONINFOW))GetProcAddress(dllHandle,"RtlGetVersion")))
        {
            if(RtlGetVersion((_RTL_OSVERSIONINFOW *) &osver))
            {
                osver.dwOSVersionInfoSize = sizeof(_RTL_OSVERSIONINFOW);
                if(RtlGetVersion((_RTL_OSVERSIONINFOW *) &osver))
                    SDDebugMessage("OSINFO::init()","RtlGetVersion is fail!");
                else
                    SDDebugMessage("OSINFO::init()","RtlGetVersion is fail for _RTL_OSVERSIONINFOEXW will use _RTL_OSVERSIONINFOW");
            }
            else
            {
                /*qDebug() << "RtlGetVersion is OK"*/;
            }
        }else
        {
            SDDebugMessage("OSINFO::init()","Can not find API RtlGetVersion() in ntdll.dll");
        }
        FreeLibrary(dllHandle);
    }else
    {
        SDDebugMessage("OSINFO::init()","LoadLibrary \"ntdll.dll\" is fail!");
    }
    if ( osver.wProductType == VER_NT_SERVER )
    {
        SDDebugMessage("OSINFO::init()","Probably server");
        isServer = true;
    }
    lastError.clear();
    is64 = isWow64();
    if (lastError.size()) SDDebugMessage("OSINFO::init()",lastError);
    isAdmin = isRunAsAdmin();
    MajorVersion = osver.dwMajorVersion;
    MinorVersion = osver.dwMinorVersion;
    BuildNumber = osver.dwBuildNumber;
    //unsigned long  dwOSVersionInfoSize;
    PlatformId = osver.dwPlatformId;
    szCSDVersion = QString::fromWCharArray(osver.szCSDVersion);
    ServicePackMajor = osver.wServicePackMajor;
    ServicePackMinor = osver.wServicePackMinor;;
    SuiteMask =  osver.wSuiteMask;
    ProductType = osver.wProductType;
    Reserved = osver.wReserved;

    int tmp;

    int win;
    win=MajorVersion*1000+MinorVersion*100+int(isServer)*10+int(is64);
    Win=static_cast<WIN>(win);
    tmp=win;
    if (is64) win--;
    if (isServer && MajorVersion == 6 && MinorVersion) win++;
    WinNoarch=static_cast<WINNOARCH>(win);

    SDDebugMessage("OSINFO::init()",QString("WIN: %1\nWINNOARCH %2").arg(tmp).arg(win));

    return MajorVersion;
}

QString verExpand(QString string)
{
    QString current = string;
    if (!current.indexOf("FROMFILE:"))
    {
//            if (OSinfo.is64)
//            {//сначала самые ходовые
//                if (OSinfo.Win >= OSINFO::W7X64)
//                {
//                    current.replace("%programfiles%","%ProgramW6432%");
//                    current.replace("%PROGRAMFILES%","%ProgramW6432%");
//                }else
//                {
//                    current.replace("%programfiles%","%SYSTEMDRIVE%\\Program Files");
//                    current.replace("%PROGRAMFILES%","%SYSTEMDRIVE%\\Program Files");
//                }
//            }
//            else
//            {
//                current.replace("%programfiles(x86)%","%PROGRAMFILES%");
//                current.replace("%PROGRAMFILES(x86)%","%PROGRAMFILES%");
//            }
        QStringRef vfile(&current, 9, current.size()-9);
        current = GetVer(ExpandEnvironmentString(vfile.toString()));
        if (!current.indexOf("ERROR:"))
        {
            current = string;
            current = GetVer(ExpandEnvironmentString(vfile.toString()));
            if (!current.indexOf("ERROR:"))
            {
                if (OSinfo.is64 && OSinfo.Win < OSINFO::W7X86)
                {
                    SDDebugMessage(QString::fromUtf8("verExpand(QString %1)").arg(string),QString::fromUtf8(
                                      "VerCheck failed! Probably it could happen if the file does not exist"
                                      " or folder \"Program Files\" has been moved. File %1").arg(current),
                                  true,iconwarning);
                }
            }
        }
    }
    return current;
}

//using namespace std;
//#include <comdef.h>
//#include <Wbemidl.h>

//# pragma comment(lib, "wbemuuid.lib")

//int uytu()
//{
//    HRESULT hres;

//    // Step 1: --------------------------------------------------
//    // Initialize COM. ------------------------------------------

//    hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
//    if (FAILED(hres))
//    {
//        cout << "Failed to initialize COM library. Error code = 0x"
//            << hex << hres << endl;
//        return 1;                  // Program has failed.
//    }

//    // Step 2: --------------------------------------------------
//    // Set general COM security levels --------------------------

//    hres =  CoInitializeSecurity(
//        NULL,
//        -1,                          // COM authentication
//        NULL,                        // Authentication services
//        NULL,                        // Reserved
//        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
//        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
//        NULL,                        // Authentication info
//        EOAC_NONE,                   // Additional capabilities
//        NULL                         // Reserved
//        );


//    if (FAILED(hres))
//    {
//        cout << "Failed to initialize security. Error code = 0x"
//            << hex << hres << endl;
//        CoUninitialize();
//        return 1;                    // Program has failed.
//    }

//    // Step 3: ---------------------------------------------------
//    // Obtain the initial locator to WMI -------------------------

//    IWbemLocator *pLoc = NULL;

//    hres = CoCreateInstance(
//        CLSID_WbemLocator,
//        0,
//        CLSCTX_INPROC_SERVER,
//        IID_IWbemLocator, (LPVOID *) &pLoc);

//    if (FAILED(hres))
//    {
//        cout << "Failed to create IWbemLocator object."
//            << " Err code = 0x"
//            << hex << hres << endl;
//        CoUninitialize();
//        return 1;                 // Program has failed.
//    }

//    // Step 4: -----------------------------------------------------
//    // Connect to WMI through the IWbemLocator::ConnectServer method

//    IWbemServices *pSvc = NULL;

//    // Connect to the root\cimv2 namespace with
//    // the current user and obtain pointer pSvc
//    // to make IWbemServices calls.
//    hres = pLoc->ConnectServer(
//         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
//         NULL,                    // User name. NULL = current user
//         NULL,                    // User password. NULL = current
//         0,                       // Locale. NULL indicates current
//         NULL,                    // Security flags.
//         0,                       // Authority (for example, Kerberos)
//         0,                       // Context object
//         &pSvc                    // pointer to IWbemServices proxy
//         );

//    if (FAILED(hres))
//    {
//        cout << "Could not connect. Error code = 0x"
//             << hex << hres << endl;
//        pLoc->Release();
//        CoUninitialize();
//        return 1;                // Program has failed.
//    }

//    cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


//    // Step 5: --------------------------------------------------
//    // Set security levels on the proxy -------------------------

//    hres = CoSetProxyBlanket(
//       pSvc,                        // Indicates the proxy to set
//       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
//       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
//       NULL,                        // Server principal name
//       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
//       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
//       NULL,                        // client identity
//       EOAC_NONE                    // proxy capabilities
//    );

//    if (FAILED(hres))
//    {
//        cout << "Could not set proxy blanket. Error code = 0x"
//            << hex << hres << endl;
//        pSvc->Release();
//        pLoc->Release();
//        CoUninitialize();
//        return 1;               // Program has failed.
//    }

//    // Step 6: --------------------------------------------------
//    // Use the IWbemServices pointer to make requests of WMI ----

//    // For example, get the name of the operating system
//    IEnumWbemClassObject* pEnumerator = NULL;
//    hres = pSvc->ExecQuery(
//        bstr_t("WQL"),
//        bstr_t("SELECT * FROM Win32_OperatingSystem"),
//        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
//        NULL,
//        &pEnumerator);

//    if (FAILED(hres))
//    {
//        cout << "Query for operating system name failed."
//            << " Error code = 0x"
//            << hex << hres << endl;
//        pSvc->Release();
//        pLoc->Release();
//        CoUninitialize();
//        return 1;               // Program has failed.
//    }

//    // Step 7: -------------------------------------------------
//    // Get the data from the query in step 6 -------------------

//    IWbemClassObject *pclsObj;
//    ULONG uReturn = 0;

//    while (pEnumerator)
//    {
//        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
//            &pclsObj, &uReturn);

//        if(0 == uReturn)
//        {
//            break;
//        }

//        VARIANT vtProp;

//        // Get the value of the Name property
//        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
//        wcout << " OS Name : " << vtProp.bstrVal << endl;
//        VariantClear(&vtProp);

//        pclsObj->Release();
//    }

//    // Cleanup
//    // ========

//    pSvc->Release();
//    pLoc->Release();
//    pEnumerator->Release();
//    if(!pclsObj) pclsObj->Release();
//    CoUninitialize();

//    return 0;   // Program successfully completed.

//}

//void INFO_GetWinVer(char* buf)
//{
//	OSVERSIONINFOEXA osvi;
//	SYSTEM_INFO si;

//	GetSystemInfo(&si);

//	buf[0] = 0x00;
//	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
//	if (GetVersionExA((LPOSVERSIONINFOA)&osvi))
//	{
//		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
//			osvi.dwMajorVersion > 4)
//		{
//			lstrcpyA(buf, "Microsoft ");

//			if (osvi.dwMajorVersion == 6)
//			{
//				if (osvi.dwMinorVersion == 0)
//				{
//					if (osvi.wProductType == VER_NT_WORKSTATION)
//					{
//						lstrcatA(buf, "Windows Vista ");
//					}
//					else
//					{
//						lstrcatA(buf, "Windows Server 2008 ");
//					}
//				}
//				else if (osvi.dwMinorVersion == 1)
//				{
//					if (osvi.wProductType == VER_NT_WORKSTATION)
//					{
//						lstrcatA(buf, "Windows 7 ");
//					}
//					else
//					{
//						lstrcatA(buf, "Windows Server 2008 R2 ");
//					}
//				}
//			}
//			else if (osvi.dwMajorVersion == 5)
//			{
//				if (osvi.dwMinorVersion == 2)
//				{
//					if (GetSystemMetrics(SM_SERVERR2))
//					{
//						lstrcatA(buf, "Windows Server 2003 R2 ");
//					}
//					else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
//					{
//						lstrcatA(buf, "Windows Storage Server 2003");
//					}
//					else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
//					{
//						lstrcatA(buf, "Windows Home Server");
//					}
//					else if (osvi.wProductType == VER_NT_WORKSTATION &&
//							si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_  AMD64)
//					{
//						lstrcatA(buf, "Windows XP Professional x64 Edition");
//					}
//					else
//					{
//						lstrcatA(buf, "Windows Server 2003 ");
//					}

//					if (osvi.wProductType != VER_NT_WORKSTATION)
//					{
//						if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
//						{
//							if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
//							{
//								lstrcatA(buf, "Datacenter Edition for Itanium-based Systems");
//							}
//							else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
//							{
//								lstrcatA(buf, "Enterprise Edition for Itanium-based Systems");
//							}
//						}
//			            else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
//						{
//							if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
//							{
//								lstrcatA(buf, "Datacenter x64 Edition");
//							}
//							else if(osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
//							{
//								lstrcatA(buf, "Enterprise x64 Edition");
//							}
//							else
//							{
//								lstrcatA(buf, "Standard x64 Edition");
//							}
//						}
//						else
//						{
//							if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
//							{
//								lstrcatA(buf, "Compute Cluster Edition");
//							}
//							else
//							{
//								if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
//								{
//									lstrcatA(buf, "Datacenter Edition");
//								}
//								else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
//								{
//									lstrcatA(buf, "Enterprise Edition");
//								}
//								else if (osvi.wSuiteMask & VER_SUITE_BLADE)
//								{
//									lstrcatA(buf, "Web Edition");
//								}
//								else
//								{
//									lstrcatA(buf, "Standard Edition");
//								}
//							}
//						}
//					}
//				}
//				else if (osvi.dwMinorVersion == 1)
//				{
//					lstrcatA(buf, "Windows XP ");
//					if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
//					{
//						lstrcatA(buf, "Home Edition");
//					}
//					else
//					{
//						lstrcatA(buf, "Professional");
//					}
//				}
//				else if (osvi.dwMinorVersion == 0)
//				{
//					lstrcatA(buf, "Windows 2000 ");

//					if (osvi.wProductType == VER_NT_WORKSTATION)
//					{
//						lstrcatA(buf, "Professional");
//					}
//					else
//					{
//						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
//						{
//							lstrcatA(buf, "Datacenter Server");
//						}
//						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
//						{
//							lstrcatA(buf, "Advanced Server");
//						}
//						else
//						{
//							lstrcatA(buf, "Server");
//						}
//					}
//				}
//			}

//			if (osvi.szCSDVersion[0] != 0x00)
//			{
//				lstrcatA(buf, " ");
//				lstrcatA(buf, osvi.szCSDVersion);
//			}

//			if (osvi.dwMajorVersion >= 6)
//			{
//				if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
//				{
//					lstrcatA(buf, " 64-bit");
//				}
//				else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
//				{
//					lstrcatA(buf, " 32-bit");
//				}
//			}
//		}
//	}
//	else
//	{
//		lstrcatA(buf, "Unknown");
//	}
//}

//Product and Channel	WPA	MPC	Expiration date
//R2 Datacenter x86 	No	69754	No
//Datacenter IA-64 	No	69769	No
//R2 Datacenter x64	No	76871	No
//Enterprise Edition 32-bit retail	Yes	69713	No
//Enterprise Edition 32-bit volume licensing	No	69713	No
//Enterprise Edition 32-bit OEM	Yes (see note)	69713 	No
//Enterprise Edition 32-bit evaluation	Yes	69763 	Yes
//Enterprise Edition 32-bit checked build 	Yes	69713	No
//Datacenter Edition 32-bit retail	Yes	69754 	No
//Datacenter Edition 32-bit OEM	Yes	69754 	No
//Standard Edition 32-bit retail	Yes	69712 	No
//Standard Edition 32-bit volume licensing	No	69712 	No
//Standard Edition 32-bit  OEM 	Yes (see note)	69712 	No
//Standard Edition 32-bit evaluation	Yes	69763 	Yes
//Web Edition 32-bit retail	Yes	69753 	No
//Web Edition 32-bit volume licensing 	No	69753	No
//Web Edition 32-bit OEM	Yes (see note)	69753	No
//Web Edition 32-bit evaluation	Yes	69763	Yes
//Enterprise Edition 64-bit retail	No	69770	No
//Enterprise Edition 64-bit volume licensing	No	69770	No
//Enterprise Edition 64-bit OEM 	No	69770	No
//Enterprise Edition 64-bit evaluation	No	69763	No
//Enterprise Edition 64-bit checked build 	No	69770	No
//Datacenter Edition 64-bit retail	No	69769	No
//Datacenter Edition 64-bit OEM 	No	69769	No













//void f()
//{
//   WSC_SECURITY_PROVIDER_HEALTH health;
//   const DWORD dwAntivirus(WSC_SECURITY_PROVIDER_ANTIVIRUS);

//   HRESULT hr = WscGetSecurityProviderHealth(dwAntivirus, &health);
//   if (FAILED(hr))
//   {
//      std::cerr << "Error " << std::hex
//                << std::showbase << hr << "\n";
//      return -1;
//   }
//   switch (health)
//   {
//      case WSC_SECURITY_PROVIDER_HEALTH_GOOD:
//         std::cout << "Antivirus health is good\n";
//         return 0;
//      case WSC_SECURITY_PROVIDER_HEALTH_NOTMONITORED:
//         std::cout << "Antivirus health is not monitored\n";
//         return 1;
//      case WSC_SECURITY_PROVIDER_HEALTH_POOR:
//         std::cout << "Antivirus health is poor\n";
//         return 2;
//      case WSC_SECURITY_PROVIDER_HEALTH_SNOOZE:
//         std::cout << "Antivirus health is snooze\n";
//         return 3;
//      default:
//         std::cout << "Unexpected antivirus health value: "
//                   << std::hex << std::showbase
//                   << health << "\n";
//         return 4;
//   }
//}


