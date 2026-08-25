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
    void init();
    void cleanup();
private Q_SLOTS:
    void createDefaultSettings();
    void saveAndLoad();
    void loadMissingCreatesDefault();
    void saveWithRules();
    void writeAndReadLog();
    void getDateFromLog_missing();
    void buildLogFileName();
    void cleanLog();
};

#endif