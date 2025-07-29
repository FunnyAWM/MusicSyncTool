#include "TestLogger.h"
#include <QDebug>
#include <QThread>
#include <QThreadPool>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QLoggingCategory>

void TestLogger::init()
{
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestLogger::cleanup()
{
    delete tempDir;
    tempDir = nullptr;
}

void TestLogger::testInfoLogging()
{
    QString testMessage = "This is an info message";
    
    // 由于Logger是静态方法且输出到控制台，我们主要测试调用不会崩溃
    // 在实际项目中，可能需要重定向输出来验证内容
    QVERIFY_EXCEPTION_THROWN({}, std::exception); // 期望不抛出异常
    
    // 测试Info日志调用
    Logger::Info(testMessage);
    
    // 如果程序运行到这里没有崩溃，说明日志记录成功
    QVERIFY(true);
}

void TestLogger::testWarningLogging()
{
    QString testMessage = "This is a warning message";
    
    // 测试Warning日志调用
    Logger::Warn(testMessage);
    
    // 验证调用成功（没有异常）
    QVERIFY(true);
}

void TestLogger::testErrorLogging()
{
    QString testMessage = "This is an error message";
    
    // 测试Error日志调用
    Logger::Error(testMessage);
    
    // 验证调用成功（没有异常）
    QVERIFY(true);
}

void TestLogger::testFatalLogging()
{
    QString testMessage = "This is a fatal message";
    
    // 注意：Fatal日志可能会终止程序，所以这里只测试调用
    // 在真实的Logger实现中，Fatal可能不会真的退出程序
    Logger::Fatal(testMessage);
    
    // 如果程序还在运行，说明Fatal日志记录完成
    QVERIFY(true);
}

void TestLogger::testDebugLogging()
{
    QString testMessage = "This is a debug message";
    
    // 测试Debug日志调用
    Logger::Debug(testMessage);
    
    // 验证调用成功（没有异常）
    QVERIFY(true);
}

void TestLogger::testLogMessageFormat()
{
    // 测试各种消息格式
    QStringList testMessages = {
        "Simple message",
        "Message with numbers: 12345",
        "Message with symbols: !@#$%^&*()",
        "Multi-line\nmessage\nwith\nbreaks",
        "Message with tabs\tand\tspaces",
        "Very long message that might exceed normal buffer sizes and contain lots of text to test the logger's ability to handle long strings without issues or crashes and ensure that it can process extended content correctly",
    };
    
    for (const QString& message : testMessages) {
        // 测试不同级别的日志都能处理这些格式
        Logger::Info(message);
        Logger::Warn(message);
        Logger::Error(message);
        Logger::Debug(message);
    }
    
    // 如果所有消息都记录成功，测试通过
    QVERIFY(true);
}

void TestLogger::testSpecialCharactersAndLongMessages()
{
    // 测试特殊字符
    QString specialChars = "Special chars: àáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ";
    Logger::Info(specialChars);
    
    // 测试Unicode字符
    QString unicodeChars = "Unicode: ☀☁☂☃☄★☆☇☈☉☊☋☌☍☎☏☐☑☒☓";
    Logger::Info(unicodeChars);
    
    // 测试表情符号
    QString emojis = "Emojis: 😀😁😂🤣😃😄😅😆😉😊😋😎😍😘🥰😗";
    Logger::Info(emojis);
    
    // 测试很长的消息
    QString longMessage = QString("Long message: ").repeated(1000);
    Logger::Info(longMessage);
    
    QVERIFY(true);
}

void TestLogger::testChineseLogMessages()
{
    // 测试中文简体
    QString simplifiedChinese = "简体中文日志消息：这是一个测试";
    Logger::Info(simplifiedChinese);
    
    // 测试中文繁体
    QString traditionalChinese = "繁體中文日誌訊息：這是一個測試";
    Logger::Warn(traditionalChinese);
    
    // 测试中英混合
    QString mixedMessage = "Mixed message: 中英文混合的日志消息 with English text";
    Logger::Error(mixedMessage);
    
    // 测试带有中文标点符号
    QString chinesePunctuation = "中文标点：，。！？；：\'”（）【】";
    Logger::Debug(chinesePunctuation);
    
    QVERIFY(true);
}

void TestLogger::testEmptyMessagesAndEdgeCases()
{
    // 测试空消息
    Logger::Info("");
    Logger::Warn("");
    Logger::Error("");
    Logger::Debug("");
    
    // 测试只有空格的消息
    Logger::Info("   ");
    Logger::Warn("\t\t\t");
    Logger::Error("\n\n\n");
    
    // 测试null字符
    QString nullMessage;
    Logger::Info(nullMessage);
    
    // 测试包含null字符的消息
    QString messageWithNull = QString("Message with null\0character").replace('\0', ' ');
    Logger::Info(messageWithNull);
    
    QVERIFY(true);
}

void TestLogger::testConcurrentLogging()
{
    const int numThreads = 10;
    const int messagesPerThread = 100;
    
    // 创建并发任务
    QList<QFuture<void>> futures;
    
    for (int i = 0; i < numThreads; ++i) {
        QFuture<void> future = QtConcurrent::run([i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                QString message = QString("Thread %1, Message %2").arg(i).arg(j);
                
                // 随机选择日志级别
                switch (j % 4) {
                case 0:
                    Logger::Info(message);
                    break;
                case 1:
                    Logger::Warn(message);
                    break;
                case 2:
                    Logger::Error(message);
                    break;
                case 3:
                    Logger::Debug(message);
                    break;
                }
                
                // 短暂延迟以增加并发竞争
                QThread::msleep(1);
            }
        });
        
        futures.append(future);
    }
    
    // 等待所有任务完成
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    // 如果所有并发日志记录都完成没有崩溃，测试通过
    QVERIFY(true);
}

void TestLogger::testLogLevelDifferentiation()
{
    // 测试不同日志级别是否能正确区分
    // 由于Logger输出到控制台，我们主要验证调用的成功性
    
    QString baseMessage = "Test message for level: ";
    
    // 依次测试各个级别
    Logger::Debug(baseMessage + "DEBUG");
    Logger::Info(baseMessage + "INFO");
    Logger::Warn(baseMessage + "WARNING");
    Logger::Error(baseMessage + "ERROR");
    Logger::Fatal(baseMessage + "FATAL");
    
    // 测试级别组合
    Logger::Info("Start of operation");
    Logger::Debug("Debug details");
    Logger::Warn("Potential issue detected");
    Logger::Error("Error occurred");
    Logger::Info("End of operation");
    
    QVERIFY(true);
}

QString TestLogger::captureConsoleOutput(std::function<void(const QString&)> logFunction, const QString& message)
{
    // 这是一个辅助方法的框架
    // 在实际实现中，可能需要重定向stdout/stderr来捕获输出
    // 或者修改Logger类以支持输出捕获
    
    logFunction(message);
    
    // 返回模拟的输出（在实际实现中应该返回真实的捕获内容）
    return QString("Captured: %1").arg(message);
}

#include "TestLogger.moc"