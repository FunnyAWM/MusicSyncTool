//
// Created by funnyawm on 25-1-30.
//

#ifndef LINUXSINGLEINSTANCE_H
#define LINUXSINGLEINSTANCE_H
#include <QCoreApplication>
#include <QString>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
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
    const int pid = getpid();
    ftruncate(fd, 0);
    result = static_cast<int>(write(fd, &pid, sizeof(pid)));
    if (result == -1) {
        qWarning() << "Failed to write lock file: " << lockFileBuilder;
        close(fd);
        return false;
    }
    return true;
}

#endif // LINUXSINGLEINSTANCE_H
