#include "TestMSTFileManager.h"

void TestMSTFileManager::isFormatSupported_allFormats() {
    const QStringList fmts = {"mp3","flac","wav","aac","ogg","wma","m4a","ape","aiff","opus"};
    for (const auto& f : fmts)
        QVERIFY2(MSTFileManager::isFormatSupported("x." + f), qPrintable(f));
}

void TestMSTFileManager::isFormatSupported_unsupported() {
    QVERIFY(!MSTFileManager::isFormatSupported("f.txt"));
    QVERIFY(!MSTFileManager::isFormatSupported("f.png"));
    QVERIFY(!MSTFileManager::isFormatSupported("f.mp4"));
}

void TestMSTFileManager::isFormatSupported_caseInsensitive() {
    QVERIFY(MSTFileManager::isFormatSupported("song.MP3"));
    QVERIFY(MSTFileManager::isFormatSupported("song.Flac"));
    QVERIFY(MSTFileManager::isFormatSupported("SONG.WAV"));
}

void TestMSTFileManager::isFormatSupported_edgeCases() {
    QVERIFY(!MSTFileManager::isFormatSupported("noextension"));
    QVERIFY(MSTFileManager::isFormatSupported("file.tar.mp3"));
    QVERIFY(!MSTFileManager::isFormatSupported("file.mp3.bak"));
}

void TestMSTFileManager::rollBackCopy_deletesBoth() {
    QTemporaryDir d;
    QVERIFY(d.isValid());
    const QString mf = d.path() + "/song.mp3";
    const QString lf = d.path() + "/song.lrc";
    { QFile f(mf); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("x"); f.close(); }
    { QFile f(lf); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("x"); f.close(); }
    QVERIFY(QFile::exists(mf)); QVERIFY(QFile::exists(lf));
    MSTFileManager::rollBackCopy(mf);
    QVERIFY(!QFile::exists(mf));
    QVERIFY(!QFile::exists(lf));
}

void TestMSTFileManager::rollBackCopy_noLrc() {
    QTemporaryDir d;
    QVERIFY(d.isValid());
    const QString mf = d.path() + "/song.flac";
    { QFile f(mf); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("x"); f.close(); }
    QVERIFY(!QFile::exists(d.path() + "/song.lrc"));
    MSTFileManager::rollBackCopy(mf);
    QVERIFY(!QFile::exists(mf));
}

void TestMSTFileManager::rollBackCopy_nonexistent() {
    MSTFileManager::rollBackCopy("/nonexistent/phantom.mp3");
    QVERIFY(true);
}

void TestMSTFileManager::isFull_smallFile() {
    QTemporaryDir d;
    QVERIFY(d.isValid());
    const QString sf = d.path() + "/tiny.mp3";
    { QFile f(sf); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("tiny"); f.close(); }
    QVERIFY(!MSTFileManager::isFull(sf, d.path()));
}

void TestMSTFileManager::getSpaceInfo_format() {
    QTemporaryDir d;
    QVERIFY(d.isValid());
    MSTFileManager mgr(d.path());
    const QString info = mgr.getSpaceInfo();
    QRegularExpression re(R"(^\d+\.\d{2}\s+(MB|GB)\s+/\s+\d+\.\d{2}\s+(MB|GB)$)");
    QVERIFY2(re.match(info).hasMatch(), qPrintable(info));
}

void TestMSTFileManager::copyable_smallFile() {
    QTemporaryDir d;
    QVERIFY(d.isValid());
    const QString sf = d.path() + "/small.wav";
    { QFile f(sf); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("audio"); f.close(); }
    MSTFileManager mgr(d.path());
    QVERIFY(mgr.copyable(sf));
}

void TestMSTFileManager::copyMusicFiles_cancelKeepsCompletedFile() {
    QTemporaryDir d;
    QVERIFY(d.isValid());
    const QString srcDir = d.path() + "/src";
    const QString dstDir = d.path() + "/dst";
    QVERIFY(QDir().mkpath(srcDir));
    QVERIFY(QDir().mkpath(dstDir));
    const QStringList names = {"a.mp3", "b.mp3", "c.mp3"};
    for (const QString& n : names) {
        QFile f(srcDir + "/" + n);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("audio");
        f.close();
    }
    // b.mp3 已存在于目标目录，会触发 onError(DUPLICATE)：借此在第一首处理完之后请求取消
    { QFile f(dstDir + "/b.mp3"); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("old"); f.close(); }

    std::atomic<bool> cancelRequested{false};
    QStringList errors;
    MSTFileManager::copyMusicFiles(srcDir, names, dstDir, true,
                                   nullptr,
                                   [&errors, &cancelRequested](const QString& name, int) {
                                       errors << name;
                                       cancelRequested.store(true);
                                   },
                                   nullptr, nullptr, nullptr,
                                   &cancelRequested);

    QVERIFY(QFile::exists(dstDir + "/a.mp3"));   // 已完成的文件保留，不回滚
    QVERIFY(!QFile::exists(dstDir + "/c.mp3"));  // 取消后不再开始新的歌曲
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors.first(), QString("b.mp3"));
}

void TestMSTFileManager::copyMusicFiles_cancelBeforeStart() {
    QTemporaryDir d;
    QVERIFY(d.isValid());
    const QString srcDir = d.path() + "/src";
    const QString dstDir = d.path() + "/dst";
    QVERIFY(QDir().mkpath(srcDir));
    QVERIFY(QDir().mkpath(dstDir));
    const QStringList names = {"x.mp3", "y.mp3"};
    for (const QString& n : names) {
        QFile f(srcDir + "/" + n);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("audio");
        f.close();
    }

    std::atomic<bool> cancelRequested{true};
    int lastProgress = -1;
    MSTFileManager::copyMusicFiles(srcDir, names, dstDir, true,
                                   [&lastProgress](const int value) { lastProgress = value; },
                                   nullptr, nullptr, nullptr, nullptr,
                                   &cancelRequested);

    QVERIFY(!QFile::exists(dstDir + "/x.mp3"));
    QVERIFY(!QFile::exists(dstDir + "/y.mp3"));
    // 取消时进度停在已处理的歌曲数（0），不应跳到 100% 造成“已全部复制”的错觉
    QCOMPARE(lastProgress, 0);
}