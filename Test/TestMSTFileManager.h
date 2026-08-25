#ifndef TESTMSTFILEMANAGER_H
#define TESTMSTFILEMANAGER_H

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Services/MSTFileManager.h"

class TestMSTFileManager : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void isFormatSupported_allFormats();
    void isFormatSupported_unsupported();
    void isFormatSupported_caseInsensitive();
    void isFormatSupported_edgeCases();
    void rollBackCopy_deletesBoth();
    void rollBackCopy_noLrc();
    void rollBackCopy_nonexistent();
    void isFull_smallFile();
    void getSpaceInfo_format();
    void copyable_smallFile();
};

#endif