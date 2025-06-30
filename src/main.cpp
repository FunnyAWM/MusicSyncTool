#include "Widgets/MusicSyncTool/MusicSyncTool.h"
#include "SingleInstance.h"

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	MusicSyncTool w;
#if defined(_WIN64) or defined(_WIN32)
	HANDLE hMutex;
	if (!singleInstance(hMutex)) {
#else
    const int fd = singleInstance();
	if (fd == -1) {
#endif
		w.popError(PET::RUNNING);
		return 1;
	}
	w.show();
	a.exec(); // NOLINT(readability-static-accessed-through-instance)
#if defined(_WIN64) or defined(_WIN32)
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	hMutex = nullptr;
#elif defined(__linux)
    close(fd);
#endif
	return 0;
}
