#include <windows.h>
#include <string>


bool isWow64()
{
    BOOL bIsWow64 = FALSE;
    typedef BOOL (APIENTRY *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    HMODULE module = GetModuleHandle(TEXT("kernel32.dll"));
    if(NULL == module)
    {
        //error("Can not GetModuleHandle from kernel32.dll\nProbably, system is X86!\nCoder id Invalid!");
        return 0;
    }
    if(fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(module, "IsWow64Process"))
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //error("API fnIsWow64Process() is FAIL!\nProbably, system is X86!\nCoder id Invalid!");
            return FALSE;
        }
    }
    else
    {
        //error("Can not find API fnIsWow64Process() in kernel32.dll\nProbably, system is X86!\nCoder id Invalid!");
        return FALSE;
    }
    return bIsWow64 != FALSE;
}

void RunWithElevation(
        HWND hwnd, // can be 0
        const std::wstring &app,
        const std::wstring &parameters = L"",
        const std::wstring &directory = L"",
        bool RunWithElevation = true)
{
    SHELLEXECUTEINFO executeInfo = {0};
    executeInfo.cbSize = sizeof(SHELLEXECUTEINFO);

    executeInfo.fMask = 0;
    executeInfo.hwnd = hwnd;
    if (RunWithElevation) executeInfo.lpVerb = L"runas";

    executeInfo.lpFile = app.c_str();
    executeInfo.lpParameters = parameters.empty()
        ? 0 : parameters.c_str();
    executeInfo.lpDirectory = directory.empty()
        ? 0 : directory.c_str();
    executeInfo.nShow = SW_NORMAL;

    ShellExecuteEx( &executeInfo );
}

std::wstring ExePath() {
    TCHAR buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    std::wstring::size_type pos = std::wstring( buffer ).find_last_of( L"\\/");
    if ( pos == std::wstring::npos )
        return L"";
    else {
        return std::wstring( buffer ).substr( 0, pos);
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    bool is64 = isWow64();
    std::wstring path = ExePath()+L"\\softodrom\\softodrom.exe";
    std::wstring dir = ExePath()+L"\\softodrom\\dll32";
    if (is64)
    {
        path = ExePath()+L"\\softodrom\\softodrom64.exe";
        dir = ExePath()+L"\\softodrom\\dll64";
    }
    OSVERSIONINFO osv;
    ZeroMemory(&osv, sizeof(OSVERSIONINFO));
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osv);
    if (osv.dwMajorVersion < 5)
    {
        MessageBox(
                    NULL,
                    (LPCWSTR)L"Для запуска софтодрома нужна OS windows 2000 или более поздняя, попробуйте воспользоваться батниками!\nНадеюсь они все ещё работают. (А потом обломайтесь узнав что они даже на 2000 не запускаются, только на XP)))",
                    (LPCWSTR)L"Unsupported Windows",
                    MB_ICONWARNING
                    );
    }
    if (osv.dwMajorVersion == 5)
    {
        if (is64) RunWithElevation(0,path,L"RunWithElevation Run64",dir,false);
        else RunWithElevation(0,path,L"RunWithElevation",dir,false);
        return 0;
    }
    if (osv.dwMajorVersion > 6)
    {
        MessageBox(
                    NULL,
                    (LPCWSTR)L"Произошла ошибка при определении вашей версии windows, или вы запускаете программу на чем то совсем новом она может работать неправильно!",
                    (LPCWSTR)L"Unsupported Windows",
                    MB_ICONWARNING
                    );
    }
    if (is64) RunWithElevation(0,path,L"RunWithElevation Run64",dir);
    else RunWithElevation(0,path,L"RunWithElevation",dir);
    return 0;
}

