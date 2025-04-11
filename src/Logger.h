//
// Created by funnyawm on 25-4-11.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <QString>
class Logger final{
public:
    static void Info(const QString&);
    static void Debug(const QString&);
    static void Warn(const QString&);
    static void Error(const QString&);
    static void Fatal(const QString&);
};
#endif //LOGGER_H
