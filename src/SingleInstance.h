//
// Created by funnyawm on 25-1-30.
//

#ifndef LINUXSINGLEINSTANCE_H
#define LINUXSINGLEINSTANCE_H
#include <QCoreApplication>
#include "Logger.h"

// 根据平台选择不同的头文件
#if defined(_WIN32) or defined(_WIN64)
#include <Windows.h>
#elif defined(__linux)
#include <QString>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#if defined(_WIN32) or defined(_WIN64)
/**
 * @brief Windows平台的单实例检查函数
 * 使用Windows互斥量(Mutex)来确保应用程序只运行一个实例
 * @param mutex 互斥量句柄的引用，用于返回创建的互斥量
 * @return 如果是第一个实例返回true，否则返回false
 */
inline bool singleInstance(HANDLE& mutex) {
    // 创建命名互斥量，名称为应用程序名称
    mutex = CreateMutex(nullptr, TRUE, QCoreApplication::applicationName().toStdWString().c_str());
    
    // 检查是否已存在同名互斥量
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);  // 关闭句柄
        return false;        // 表示已有实例在运行
    }
    return true;  // 表示这是第一个实例
}

#elif defined(__linux)
/**
 * @brief Linux平台的单实例检查函数
 * 使用文件锁来确保应用程序只运行一个实例
 * @return 成功时返回文件描述符，失败时返回-1
 */
inline int singleInstance() {
    // 构建锁文件路径：/tmp/应用程序名.lock
    QString lockFileBuilder = "/tmp/";
    lockFileBuilder += QCoreApplication::applicationName();
    lockFileBuilder += ".lock";
    
    // 创建或打开锁文件
    const int fd = open(lockFileBuilder.toStdString().c_str(), O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        Logger::Warn("Failed to open lock file: " + lockFileBuilder);
        return -1;
    }
    
    // 设置文件锁结构
    flock lock{};
    lock.l_type = F_WRLCK;    // 写锁类型
    lock.l_whence = SEEK_SET; // 从文件开始位置
    lock.l_start = 0;         // 偏移量为0
    lock.l_len = 0;           // 锁定整个文件
    
    // 尝试获取文件锁
    int result = fcntl(fd, F_SETLK, &lock);
    if (result == -1) {
        Logger::Warn("Failed to lock file: " + lockFileBuilder);
        close(fd);
        return -1;  // 锁定失败，表示已有实例在运行
    }
    
    // 截断文件并写入当前进程ID
    ftruncate(fd, 0);
    result = static_cast<int>(write(fd, QString::number(getpid()).toStdString().c_str(), 
                                   QString::number(getpid()).length()));
    if (result == -1) {
        Logger::Warn("Failed to write lock file: " + lockFileBuilder);
        close(fd);
        return -1;
    }
    
    return fd;  // 返回文件描述符，表示成功获取锁
}
#endif

#endif // LINUXSINGLEINSTANCE_H
