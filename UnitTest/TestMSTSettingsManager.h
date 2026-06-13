#ifndef TESTMSTSETTINGSMANAGER_H
#define TESTMSTSETTINGSMANAGER_H

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Core/SettingEntity.h"
#include "../src/Services/MSTSettingsManager.h"

class TestMSTSettingsManager : public QObject {
    Q_OBJECT

private:
    QTemporaryDir *tempDir = nullptr;
    QString originalWorkDir;

private Q_SLOTS:
    void init();
    void cleanup();
    void testCreateDefaultSettings();
    void testSaveAndLoadSettings();
    void testLoadSettings_missingFile_createsDefault();
    void testSaveSettings_withRules();
    void testSaveSettings_emptyFavoriteTag();
    void testWriteAndReadLog();
    void testGetDateFromLog_missingFile();
    void testBuildLogFileName_normalPath();
    void testBuildLogFileName_pathWithColon();
    void testCleanLog();
};

#endif // TESTMSTSETTINGSMANAGER_H
