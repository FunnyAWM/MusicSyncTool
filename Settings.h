#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QJsonArray>
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
    void setIgnoreLyricToUI(bool);
    void setSortByToUI(short);
    void setLanguageToUI(QString);
    void setFavoriteTagToUI(QString);
    void setOrderByToUI(short);
    ~Settings() = default;
public slots:
    void on_confirmButton_clicked();

private:
    Ui::SettingsClass ui;
    set entity;
signals:
    void confirmPressed(set entity);
};

#endif // SETTINGS_H
