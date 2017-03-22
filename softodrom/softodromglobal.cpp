#include <windows.h>
#include <iostream>
#include "softodromglobal.h"
#include "errcodes.h"
#include <ctime>

QSettings *AppSettings;
OSINFO OSinfo;
volatile unsigned long semaphore;


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
    kit.replace("avir",QString::fromUtf8("Антивирусы"));
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
    std::cerr << "\n" << QString::number(clock()).toStdString().c_str() << " "
              << QString(header.toLocal8Bit()).toStdString().c_str();
    if (text.size()) std::cerr << " MESSAGE: " << QString(text.toLocal8Bit()).toStdString().c_str() << "\n";

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
    if (!OSinfo.is64)
    {
        str.replace("%programfiles(x86)%","%PROGRAMFILES%");
        str.replace("%PROGRAMFILES(x86)%","%PROGRAMFILES%");
    }
    TCHAR szEnvPath[MAX_PATH];
    ExpandEnvironmentStrings(str.toStdWString().c_str(), szEnvPath, MAX_PATH );
    return QString::fromWCharArray(szEnvPath);

}

SdVersion GetVer(QString fileName)
{
    if (!fileName.indexOf("FROMFILE:"))
    {
        fileName = QStringRef(&fileName, 9, fileName.size()-9).toString();
    }
    WCHAR strTmp[512];
    DWORD handle;         //didn't actually use (dummy var)
    int infoSize = (int) GetFileVersionInfoSize(fileName.toStdWString().c_str(), &handle);
    if(infoSize == 0) return SdVersion();"ERROR: Function <GetFileVersionInfoSize> unsuccessful!";
    LPVOID pBlock;
    pBlock = new BYTE[infoSize];
    int bResult = GetFileVersionInfo(fileName.toStdWString().c_str(), handle, infoSize, pBlock);
    if(bResult == 0)
    {
        return SdVersion();"ERROR: Function <GetFileVersionInfo> unsuccessful!";
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
        return SdVersion();"ERROR: Function <VerQueryValue> for Translation unsuccessful!";
    }
    langNumber = dwBytes/sizeof(struct LANGANDCODEPAGE);
    //langNumber always must be equal 1
    if(langNumber != 1)
    {
        return SdVersion();QString("ERROR: Languages number: %1.").arg(langNumber);
    }
    HRESULT hr;
    for(int i=0; i<langNumber; i++)
    {
        hr = wsprintfW(strTmp, TEXT("\\StringFileInfo\\%04X%04X\\FileVersion"),lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
        if (FAILED(hr))
        {
            return SdVersion();"ERROR: Unable to get information in this language!";
        }
        // Retrieve file description for language and code page "i".

        if (VerQueryValue(pBlock, L"\\", &lpBuffer, &dwBytes))
        {
            VS_FIXEDFILEINFO *fi = (VS_FIXEDFILEINFO*)lpBuffer;
            return SdVersion(HIWORD(fi->dwFileVersionMS),LOWORD(fi->dwFileVersionMS),HIWORD(fi->dwFileVersionLS),LOWORD(fi->dwFileVersionLS));
            //fileName.clear();//теперь это версия %)
            //QTextStream(&fileName) << HIWORD(fi->dwFileVersionMS) << "." << LOWORD(fi->dwFileVersionMS) << "." << HIWORD(fi->dwFileVersionLS) << "." << LOWORD(fi->dwFileVersionLS);
        }else
        {
            return SdVersion();"ERROR: Function <VerQueryValue> for StringFileInfo unsuccessful!";
        }
    }
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
        case W10P: format = format.replace("%Name%","10"); break;
        case W2014P: format = format.replace("%Name%","Technical Preview"); break;
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
    isWine = 0;
    _RTL_OSVERSIONINFOEXW osver;
    ZeroMemory(&osver, sizeof(_RTL_OSVERSIONINFOEXW));
    osver.dwOSVersionInfoSize = sizeof(_RTL_OSVERSIONINFOEXW);

    HINSTANCE dllHandle;
    if ((dllHandle = LoadLibrary(TEXT("ntdll.dll"))))
    {
        static const char (*pwine_get_version)(void);
        pwine_get_version = (const char (*)())GetProcAddress(dllHandle, "wine_get_version");
        if(pwine_get_version)
        {
            isWine = true;
            wineVer = pwine_get_version();
        }
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
        QStringRef vfile(&current, 9, current.size()-9);
        SDDebugMessage(QString::fromUtf8("verExpand(QString %1)").arg(string),QString::fromUtf8(
                          "Expand file %1").arg(ExpandEnvironmentString(vfile.toString())));
        //current = GetVer(ExpandEnvironmentString(vfile.toString()));
        if (!current.indexOf("ERROR:"))
        {
            SDDebugMessage(QString::fromUtf8("verExpand(QString %1)").arg(string),QString::fromUtf8(
                              "%1").arg(current));
            current = string;
            //current = GetVer(ExpandEnvironmentString(vfile.toString()));
        }
    }
    return current;
}

bool copyDir(const QString &src, const QString &dest)
{
    SDDebugMessage(QString("copyDir(%1, %2)").arg(src).arg(dest),"Started");
    QDir dir(src);
    QDir dirdest(dest);
    if (!dir.isReadable()) return false;
    if (!dirdest.exists()) dirdest.mkdir(".");
    foreach (QFileInfo finfo, dir.entryInfoList())
    {
        SDDebugMessage(QString("copyDir(%1, %2)").arg(src).arg(dest),"for "+finfo.fileName());
        if(finfo.fileName()=="." || finfo.fileName()=="..") continue;
        if(finfo.isDir())
        {
            SDDebugMessage(QString("copyDir(%1, %2)").arg(src).arg(dest),"recursive ("+finfo.filePath()+","+dest+QDir::separator()+finfo.fileName()+")");
            if (!copyDir(finfo.absolutePath(),dest+QDir::separator()+finfo.fileName())) return false;
            continue;
        }
        if(finfo.isSymLink())
        {
            /* do something here */
            SDDebugMessage(QString("copyDir(%1, %2)").arg(src).arg(dest),"symlink ("+dest+finfo.fileName()+")");
            continue;
        }
        QFile file(finfo.filePath());
        if(finfo.isFile())
        {
            SDDebugMessage(QString("copyDir(%1, %2)").arg(src).arg(dest),"copy to "+dirdest.absolutePath()+QDir::separator()+finfo.fileName());
            if (!file.copy(dirdest.absolutePath()+QDir::separator()+finfo.fileName())) return false;
        }
        else return false;
    }
    return true;
}

