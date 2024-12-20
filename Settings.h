#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTranslator>
#include <QWidget>
#include "MusicProperties.h"
#include "SettingEntity.h"
#include "ui_Settings.h"

using TOOLPROPERTIES::orderByEnum;
using TOOLPROPERTIES::sortByEnum;

class Settings : public QWidget {
    Q_OBJECT

public:
    Settings(QWidget *parent = nullptr);
    set getSettings();
    void setIgnoreLyric(bool);
    void setSortBy(short);
    void setLanguage(QString);
    void setFavoriteTag(QString);
    void setOrderBy(short);
    ~Settings();
public slots:
    void on_confirmButton_clicked();

private:
    Ui::SettingsClass ui;
    set entity;
signals:
    void confirmPressed(set entity);
};

#endif // SETTINGS_H
