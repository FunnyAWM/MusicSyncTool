#include "Core/SingleInstance.h"
#include "Services/Logger.h"
#include "Services/MSTSettingsManager.h"
#include "UI/MusicSyncTool/MSTMainWindow.h"

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);

	// 先加载设置：主题需要在创建窗口前应用，日志开关需要在产生日志前生效
	SettingsData entity;
	if (!MSTSettingsManager::loadSettings(entity)) {
		Logger::Warn("Settings file missing or invalid, default settings created");
	}
	Logger::setLogToFile(entity.logToFile);
	MSTMainWindow::applyTheme(entity);

	MSTMainWindow w;
#if defined(_WIN64) or defined(_WIN32)
	HANDLE hMutex;
	if (!singleInstance(hMutex)) {
#else
    const int fd = singleInstance();
	if (fd == -1) {
#endif
		w.popError(AppErrorType::RUNNING);
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