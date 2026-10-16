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
	// 单实例载体按平台分派：Windows 为互斥量句柄，Linux/macOS 为锁文件描述符
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hMutex = nullptr;
	const bool isFirstInstance = singleInstance(hMutex);
#elif defined(__linux__) || defined(__APPLE__)
	const int fd = singleInstance();
	const bool isFirstInstance = fd != -1;
#endif
	if (!isFirstInstance) {
		w.popError(AppErrorType::RUNNING);
		return 1;
	}
	w.show();
	a.exec(); // NOLINT(readability-static-accessed-through-instance)
#if defined(_WIN32) || defined(_WIN64)
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	hMutex = nullptr;
#elif defined(__linux__) || defined(__APPLE__)
	close(fd);
#endif
	return 0;
}