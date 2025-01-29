#include "MusicSyncTool.h"
#if defined(_WIN64) or defined(_WIN32)
#include <Windows.h>
#endif

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MusicSyncTool w;
#if defined(_WIN64) or defined(_WIN32)
    HANDLE hMutex = CreateMutex(nullptr, TRUE, qApp->applicationName().toStdWString().c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        w.popError(PET::RUNNING);
        if (hMutex) {
            CloseHandle(hMutex);
            hMutex = nullptr;
        }
        return 1;
    }
#endif
    w.show();
    a.exec(); // NOLINT(readability-static-accessed-through-instance)
#if defined(_WIN64) or defined(_WIN32)
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    hMutex = nullptr;
#endif
    return 0;
}
