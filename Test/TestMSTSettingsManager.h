#ifndef TESTMSTSETTINGSMANAGER_H
#define TESTMSTSETTINGSMANAGER_H

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Core/SettingEntity.h"
#include "../src/Services/MSTSettingsManager.h"

class TestMSTSettingsManager : public QObject {
    Q_OBJECT
private:
    QTemporaryDir *tmp = nullptr;
    QString origCwd;
private Q_SLOTS:
    // QTest 只调用私有槽形式的 init/cleanup，声明在 private: 下会被静默忽略，夹具将从不执行
    void init();
    void cleanup();
    void createDefaultSettings();
    void saveAndLoad();
    void loadMissingCreatesDefault();
    void saveWithRules();
    void writeAndReadLog();
    void getDateFromLog_missing();
    void buildLogFileName();
    void cleanLog();
    void defaultsIncludeThemeAndLog();
    void saveAndLoadThemeAndLog();
    void legacyFileKeepsDefaults();
};

#endif