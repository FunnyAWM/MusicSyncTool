#include "TestMSTFileManager.h"
#include <QDir>
#include <QStorageInfo>

void TestMSTFileManager::init()
{
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestMSTFileManager::cleanup()
{
    delete tempDir;
    tempDir = nullptr;
}

void TestMSTFileManager::testIsFormatSupported()
{
    // 测试支持的格式
    QVERIFY(MSTFileManager::isFormatSupported("test.mp3"));
    QVERIFY(MSTFileManager::isFormatSupported("music.flac"));
    QVERIFY(MSTFileManager::isFormatSupported("song.wav"));
    QVERIFY(MSTFileManager::isFormatSupported("audio.aac"));
    QVERIFY(MSTFileManager::isFormatSupported("track.ogg"));
    QVERIFY(MSTFileManager::isFormatSupported("music.wma"));
    QVERIFY(MSTFileManager::isFormatSupported("song.m4a"));
    QVERIFY(MSTFileManager::isFormatSupported("audio.ape"));
    QVERIFY(MSTFileManager::isFormatSupported("track.aiff"));
    QVERIFY(MSTFileManager::isFormatSupported("music.opus"));
    
    // 测试不支持的格式
    QVERIFY(!MSTFileManager::isFormatSupported("document.txt"));
    QVERIFY(!MSTFileManager::isFormatSupported("video.mp4"));
    QVERIFY(!MSTFileManager::isFormatSupported("image.jpg"));
    QVERIFY(!MSTFileManager::isFormatSupported("archive.zip"));
    QVERIFY(!MSTFileManager::isFormatSupported("executable.exe"));
    
    // 测试复杂路径
    QVERIFY(MSTFileManager::isFormatSupported("/path/to/music.mp3"));
    QVERIFY(MSTFileManager::isFormatSupported("C:\\Music\\song.flac"));
    QVERIFY(!MSTFileManager::isFormatSupported("/path/to/document.pdf"));
}

void TestMSTFileManager::testSupportedFormatList()
{
    const QStringList& formats = MSTFileManager::supportedFormat;
    
    // 验证包含主要音频格式
    QVERIFY(formats.contains("mp3"));
    QVERIFY(formats.contains("flac"));
    QVERIFY(formats.contains("wav"));
    QVERIFY(formats.contains("aac"));
    QVERIFY(formats.contains("ogg"));
    QVERIFY(formats.contains("wma"));
    QVERIFY(formats.contains("m4a"));
    QVERIFY(formats.contains("ape"));
    QVERIFY(formats.contains("aiff"));
    QVERIFY(formats.contains("opus"));
    
    // 验证格式数量
    QVERIFY(formats.size() >= 10);
    
    // 验证不包含非音频格式
    QVERIFY(!formats.contains("txt"));
    QVERIFY(!formats.contains("jpg"));
    QVERIFY(!formats.contains("mp4"));
}

void TestMSTFileManager::testFormatCaseInsensitive()
{
    // 测试大写扩展名
    QVERIFY(MSTFileManager::isFormatSupported("music.MP3"));
    QVERIFY(MSTFileManager::isFormatSupported("song.FLAC"));
    QVERIFY(MSTFileManager::isFormatSupported("audio.WAV"));
    
    // 测试混合大小写
    QVERIFY(MSTFileManager::isFormatSupported("track.Mp3"));
    QVERIFY(MSTFileManager::isFormatSupported("music.FLaC"));
    QVERIFY(MSTFileManager::isFormatSupported("song.WaV"));
    
    // 测试不支持的格式（大小写变体）
    QVERIFY(!MSTFileManager::isFormatSupported("document.TXT"));
    QVERIFY(!MSTFileManager::isFormatSupported("video.Mp4"));
}

void TestMSTFileManager::testIsFull()
{
    // 创建一个小测试文件
    QString testFilePath = tempDir->filePath("test.txt");
    QFile testFile(testFilePath);
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    
    // 写入少量数据（1KB）
    QByteArray data(1024, 'A');
    testFile.write(data);
    testFile.close();
    
    // 测试当前目录（应该有足够空间）
    QVERIFY(!MSTFileManager::isFull(testFilePath, tempDir->path()));
    
    // 测试无效路径
    QVERIFY(MSTFileManager::isFull(testFilePath, "/nonexistent/path"));
}

void TestMSTFileManager::testRollBackCopy()
{
    // 创建测试文件
    QString musicFile = tempDir->filePath("test.mp3");
    QString lyricFile = tempDir->filePath("test.lrc");
    
    QFile music(musicFile);
    QFile lyric(lyricFile);
    
    QVERIFY(music.open(QIODevice::WriteOnly));
    QVERIFY(lyric.open(QIODevice::WriteOnly));
    
    music.write("test music data");
    lyric.write("test lyric data");
    music.close();
    lyric.close();
    
    // 验证文件存在
    QVERIFY(QFile::exists(musicFile));
    QVERIFY(QFile::exists(lyricFile));
    
    // 执行回滚
    MSTFileManager::rollBackCopy(musicFile);
    
    // 验证文件被删除
    QVERIFY(!QFile::exists(musicFile));
    QVERIFY(!QFile::exists(lyricFile));
}

void TestMSTFileManager::testConstructorAndBasicFunctions()
{
    MSTFileManager manager(tempDir->path());
    
    // 测试获取空间信息
    QString spaceInfo = manager.getSpaceInfo();
    QVERIFY(!spaceInfo.isEmpty());
    
    // 空间信息应该包含 "MB" 或 "GB"
    QVERIFY(spaceInfo.contains("MB") || spaceInfo.contains("GB"));
    
    // 应该包含斜杠分隔符
    QVERIFY(spaceInfo.contains(" / "));
}

void TestMSTFileManager::testCopyable()
{
    MSTFileManager manager(tempDir->path());
    
    // 创建一个小测试文件
    QString testFilePath = tempDir->filePath("small.txt");
    QFile testFile(testFilePath);
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    testFile.write("small data");
    testFile.close();
    
    // 小文件应该可以复制
    QVERIFY(manager.copyable(testFilePath));
    
    // 测试不存在的文件
    QVERIFY(!manager.copyable(tempDir->filePath("nonexistent.txt")));
    
    // 测试目录而不是文件
    QVERIFY(!manager.copyable(tempDir->path()));
}

void TestMSTFileManager::testGetSpaceInfo()
{
    MSTFileManager manager(tempDir->path());
    QString spaceInfo = manager.getSpaceInfo();
    
    // 检查格式：应该是 "数字 单位 / 数字 单位"
    QStringList parts = spaceInfo.split(" / ");
    QCOMPARE(parts.size(), 2);
    
    // 检查可用空间部分
    QString availablePart = parts[0];
    QVERIFY(availablePart.contains("MB") || availablePart.contains("GB"));
    
    // 检查总空间部分
    QString totalPart = parts[1];
    QVERIFY(totalPart.contains("MB") || totalPart.contains("GB"));
    
    // 检查数字部分是有效的
    QString availableNumStr = availablePart.split(" ")[0];
    QString totalNumStr = totalPart.split(" ")[0];
    
    bool ok1, ok2;
    double availableNum = availableNumStr.toDouble(&ok1);
    double totalNum = totalNumStr.toDouble(&ok2);
    
    QVERIFY(ok1 && ok2);
    QVERIFY(availableNum >= 0);
    QVERIFY(totalNum > 0);
    QVERIFY(availableNum <= totalNum);
}

void TestMSTFileManager::testEdgeCases()
{
    // 测试空文件名
    QVERIFY(!MSTFileManager::isFormatSupported(""));
    
    // 测试只有扩展名的文件
    QVERIFY(MSTFileManager::isFormatSupported(".mp3"));
    
    // 测试没有扩展名的文件
    QVERIFY(!MSTFileManager::isFormatSupported("filename"));
    
    // 测试多个点的文件名
    QVERIFY(MSTFileManager::isFormatSupported("file.name.with.dots.mp3"));
    QVERIFY(!MSTFileManager::isFormatSupported("file.name.with.dots.txt"));
    
    // 测试回滚不存在的文件
    QString nonExistentFile = tempDir->filePath("nonexistent.mp3");
    // 这应该不会崩溃
    MSTFileManager::rollBackCopy(nonExistentFile);
    
    // 测试无效路径的磁盘检查
    QVERIFY(MSTFileManager::isFull("nonexistent.txt", ""));
    QVERIFY(MSTFileManager::isFull("", tempDir->path()));
}

#include "TestMSTFileManager.moc"
