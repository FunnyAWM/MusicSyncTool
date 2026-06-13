/**
 * @file TestMSTFileManager.cpp
 * @brief Unit tests for MSTFileManager (Services layer)
 * @details Tests isFormatSupported, rollBackCopy, isFull, getSpaceInfo, and copyable.
 *          Uses QTemporaryDir for all file I/O so tests are self-contained and
 *          cross-platform (Windows / Linux).
 */

#include "TestMSTFileManager.h"

// ======================== isFormatSupported ========================

void TestMSTFileManager::testIsFormatSupported_mp3()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.mp3"));
}

void TestMSTFileManager::testIsFormatSupported_flac()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.flac"));
}

void TestMSTFileManager::testIsFormatSupported_wav()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.wav"));
}

void TestMSTFileManager::testIsFormatSupported_aac()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.aac"));
}

void TestMSTFileManager::testIsFormatSupported_ogg()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.ogg"));
}

void TestMSTFileManager::testIsFormatSupported_wma()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.wma"));
}

void TestMSTFileManager::testIsFormatSupported_m4a()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.m4a"));
}

void TestMSTFileManager::testIsFormatSupported_ape()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.ape"));
}

void TestMSTFileManager::testIsFormatSupported_aiff()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.aiff"));
}

void TestMSTFileManager::testIsFormatSupported_opus()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.opus"));
}

void TestMSTFileManager::testIsFormatSupported_unsupported()
{
    QVERIFY(!MSTFileManager::isFormatSupported("document.txt"));
    QVERIFY(!MSTFileManager::isFormatSupported("image.png"));
    QVERIFY(!MSTFileManager::isFormatSupported("archive.zip"));
    QVERIFY(!MSTFileManager::isFormatSupported("video.mp4"));
}

void TestMSTFileManager::testIsFormatSupported_caseInsensitive()
{
    // Implementation uses Qt::CaseInsensitive comparison
    QVERIFY(MSTFileManager::isFormatSupported("song.MP3"));
    QVERIFY(MSTFileManager::isFormatSupported("song.Flac"));
    QVERIFY(MSTFileManager::isFormatSupported("song.WAV"));
    QVERIFY(MSTFileManager::isFormatSupported("song.AAC"));
    QVERIFY(MSTFileManager::isFormatSupported("song.Ogg"));
    QVERIFY(MSTFileManager::isFormatSupported("SONG.M4A"));
}

void TestMSTFileManager::testIsFormatSupported_noExtension()
{
    // fileName.section('.', -1) returns the whole string when there is no dot
    QVERIFY(!MSTFileManager::isFormatSupported("noextension"));
}

void TestMSTFileManager::testIsFormatSupported_doubleExtension()
{
    // section('.', -1) returns the last segment after the final dot
    QVERIFY(MSTFileManager::isFormatSupported("file.tar.mp3"));
    QVERIFY(!MSTFileManager::isFormatSupported("file.mp3.bak"));
}

// ======================== rollBackCopy ========================

void TestMSTFileManager::testRollBackCopy_deletesMusicFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString musicFile = tempDir.path() + "/test_song.mp3";
    QFile f(musicFile);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("fake audio data");
    f.close();
    QVERIFY(QFile::exists(musicFile));

    MSTFileManager::rollBackCopy(musicFile);

    QVERIFY(!QFile::exists(musicFile));
}

void TestMSTFileManager::testRollBackCopy_deletesLrcFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Implementation: lyric = fileName.section('.', 0, -2) + ".lrc"
    // For "song.mp3" -> section('.', 0, -2) = "song" -> lyric = "song.lrc"
    const QString musicFile = tempDir.path() + "/test_song.mp3";
    const QString lrcFile   = tempDir.path() + "/test_song.lrc";

    QFile mf(musicFile);
    QVERIFY(mf.open(QIODevice::WriteOnly));
    mf.write("fake audio data");
    mf.close();

    QFile lf(lrcFile);
    QVERIFY(lf.open(QIODevice::WriteOnly));
    lf.write("fake lyric data");
    lf.close();

    QVERIFY(QFile::exists(musicFile));
    QVERIFY(QFile::exists(lrcFile));

    MSTFileManager::rollBackCopy(musicFile);

    QVERIFY2(!QFile::exists(musicFile), "Music file should be deleted by rollback");
    QVERIFY2(!QFile::exists(lrcFile),   "LRC file should be deleted by rollback");
}

void TestMSTFileManager::testRollBackCopy_noLrcFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString musicFile = tempDir.path() + "/test_song.flac";
    const QString lrcFile   = tempDir.path() + "/test_song.lrc";

    QFile mf(musicFile);
    QVERIFY(mf.open(QIODevice::WriteOnly));
    mf.write("fake audio data");
    mf.close();

    QVERIFY(!QFile::exists(lrcFile)); // No .lrc file present

    MSTFileManager::rollBackCopy(musicFile); // Should not crash

    QVERIFY(!QFile::exists(musicFile));
}

void TestMSTFileManager::testRollBackCopy_nonexistentFile()
{
    // rollBackCopy on a path that does not exist should not crash
    MSTFileManager::rollBackCopy("/nonexistent/path/phantom.mp3");
    QVERIFY(true);
}

// ======================== isFull ========================

void TestMSTFileManager::testIsFull_smallFile_largeTarget()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Create a tiny source file
    const QString smallFile = tempDir.path() + "/tiny.mp3";
    QFile f(smallFile);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("small data");
    f.close();

    // The temp directory resides on a disk with far more free space than 10 bytes
    QVERIFY2(!MSTFileManager::isFull(smallFile, tempDir.path()),
             "A 10-byte file should not fill the disk");
}

// ======================== getSpaceInfo ========================

void TestMSTFileManager::testGetSpaceInfo_format()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    MSTFileManager mgr(tempDir.path());
    const QString info = mgr.getSpaceInfo();

    // Expected format: "X.XX MB / Y.YY MB"  OR  "X.XX GB / Y.YY GB"
    const QRegularExpression re(
        R"(^\d+\.\d{2}\s+(MB|GB)\s+/\s+\d+\.\d{2}\s+(MB|GB)$)");
    const auto match = re.match(info);

    QVERIFY2(match.hasMatch(),
             qPrintable(QString("getSpaceInfo format mismatch: \"%1\"").arg(info)));

    // Both units must be identical (both MB or both GB)
    QCOMPARE(match.captured(1), match.captured(2));
}

// ======================== copyable ========================

void TestMSTFileManager::testCopyable_smallFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString smallFile = tempDir.path() + "/small.wav";
    QFile f(smallFile);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("tiny audio content");
    f.close();

    MSTFileManager mgr(tempDir.path());
    QVERIFY2(mgr.copyable(smallFile),
             "A small file on a disk with plenty of space should be copyable");
}
