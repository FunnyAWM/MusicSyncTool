/**
 * @file main.cpp
 * @brief 音乐同步工具应用程序的主入口文件
 * @details 包含应用程序的main函数，负责初始化Qt应用程序、
 *          创建主窗口、处理单实例运行控制和资源清理
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "Core/SingleInstance.h"
#include "UI/MusicSyncTool/MSTMainWindow.h"

#include <QFile>

/**
 * @brief 应用程序主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出状态码，0表示正常退出，1表示异常退出
 * @details 程序的主入口函数，执行以下操作：
 *          1. 初始化Qt应用程序实例
 *          2. 创建音乐同步工具主窗口实例
 *          3. 检查单实例运行（防止重复启动）
 *          4. 显示主窗口并进入事件循环
 *          5. 程序退出时清理资源（互斥锁/文件描述符）
 *          
 *          支持Windows和Linux平台的单实例控制：
 *          - Windows: 使用Mutex互斥锁
 *          - Linux: 使用文件锁机制
 */
int main(int argc, char* argv[]) {
	QApplication a(argc, argv);

	// Load stylesheets in order: base → mode → accent theme
	// 1. Base: layout, sizing, font, shared neutral properties
	// 2. Mode: background, text, border colors (light or dark)
	// 3. Theme: button & accent element colors only
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
