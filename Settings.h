#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include "ui_Settings.h"
#include "SettingEntity.h"
#include "MusicProperties.h"
#include <QFile>
#include <QTranslator>
#include <QJsonDocument>
#include <QJsonObject>

using TOOLPROPERTIES::sortByEnum;
using TOOLPROPERTIES::orderByEnum;

class Settings : public QWidget
{
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
