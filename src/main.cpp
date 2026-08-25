#include "Core/SingleInstance.h"
#include "UI/MusicSyncTool/MSTMainWindow.h"

#include <QFile>

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);

	// Load stylesheets in order: base → mode → accent theme

	QFile baseStyle(":/MusicSyncTool/base-style.qss");
	QString styleSheet;
	if (baseStyle.open(QFile::ReadOnly | QFile::Text)) {
		styleSheet = baseStyle.readAll();
		baseStyle.close();
	}

	// Mode: style-light.qss or style-dark-mode.qss
	QFile modeStyle(":/MusicSyncTool/style-light.qss");
	if (modeStyle.open(QFile::ReadOnly | QFile::Text)) {
		styleSheet += "\n" + modeStyle.readAll();
		modeStyle.close();
	}

	// Accent theme: style.qss (blue), style-green.qss, style-purple.qss, style-dark.qss
	QFile themeStyle(":/MusicSyncTool/style-green.qss");
	if (themeStyle.open(QFile::ReadOnly | QFile::Text)) {
		styleSheet += "\n" + themeStyle.readAll();
		themeStyle.close();
	}

	a.setStyleSheet(styleSheet);

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