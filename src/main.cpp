/**
 * @file main.cpp
 * @brief 音乐同步工具应用程序的主入口文件
 * @details 包含应用程序的main函数，负责初始化Qt应用程序、
 *          创建主窗口、处理单实例运行控制和资源清理
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "Widgets/MusicSyncTool/MusicSyncTool.h"
#include "SingleInstance.h"

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
