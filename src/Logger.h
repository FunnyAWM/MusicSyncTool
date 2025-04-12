//
// Created by funnyawm on 25-4-11.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <QString>

#include "MusicProperties.h"

class Logger final{
	static PROPERTIES::LogToFile logToFile;
public:
	static void setLogToFile(const PROPERTIES::LogToFile& logToFile) { Logger::logToFile = logToFile; }
    static void Info(const QString&);
    static void Debug(const QString&);
    static void Warn(const QString&);
    static void Error(const QString&);
    static void Fatal(const QString&);
};

#endif //LOGGER_H
