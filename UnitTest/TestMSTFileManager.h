#ifndef TESTMSTFILEMANAGER_H
#define TESTMSTFILEMANAGER_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Services/MSTFileManager.h"

class TestMSTFileManager : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testIsFormatSupported_mp3();
    void testIsFormatSupported_flac();
    void testIsFormatSupported_wav();
    void testIsFormatSupported_aac();
    void testIsFormatSupported_ogg();
    void testIsFormatSupported_wma();
    void testIsFormatSupported_m4a();
    void testIsFormatSupported_ape();
    void testIsFormatSupported_aiff();
    void testIsFormatSupported_opus();
    void testIsFormatSupported_unsupported();
    void testIsFormatSupported_caseInsensitive();
    void testIsFormatSupported_noExtension();
    void testIsFormatSupported_doubleExtension();
    void testRollBackCopy_deletesMusicFile();
    void testRollBackCopy_deletesLrcFile();
    void testRollBackCopy_noLrcFile();
    void testRollBackCopy_nonexistentFile();
    void testIsFull_smallFile_largeTarget();
    void testGetSpaceInfo_format();
    void testCopyable_smallFile();
};

#endif // TESTMSTFILEMANAGER_H
