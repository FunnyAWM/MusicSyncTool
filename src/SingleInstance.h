//
// Created by funnyawm on 25-1-30.
//

#ifndef LINUXSINGLEINSTANCE_H
#define LINUXSINGLEINSTANCE_H
#include <QCoreApplication>
#if defined(_WIN32) or defined(_WIN64)
#include <Windows.h>
#else 
#include <QString>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#if defined(_WIN32) or defined(_WIN64)
inline bool singleInstance(HANDLE& mutex) {
    mutex = CreateMutex(nullptr, TRUE, QCoreApplication::applicationName().toStdWString().c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        return false;
    }
    return true;
}
#else 
inline bool singleInstance() {
    QString lockFileBuilder = "/tmp/";
    lockFileBuilder += QCoreApplication::applicationName();
    lockFileBuilder += ".lock";
    const int fd = open(lockFileBuilder.toStdString().c_str(), O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        qWarning() << "Failed to open lock file: " << lockFileBuilder;
        return false;
    }
    flock lock{};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    int result = fcntl(fd, F_SETLK, &lock);
    if (result == -1) {
        qWarning() << "Failed to lock file: " << lockFileBuilder;
        return false;
    }
    ftruncate(fd, 0);
    result = static_cast<int>(write(fd, nullptr, 0));
    if (result == -1) {
        qWarning() << "Failed to write lock file: " << lockFileBuilder;
        close(fd);
        return false;
    }
    return true;
}
#endif
#endif // LINUXSINGLEINSTANCE_H
