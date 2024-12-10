#pragma once

#include <QWidget>
#include "ui_Settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

class Settings : public QWidget
{
	Q_OBJECT

public:
	struct set {
		bool ignoreLyric;
		int sortBy;
		QString language;
	};
	Settings(QWidget *parent = nullptr);
	constexpr static int TITLE = 1;
	constexpr static int ARTIST = 2;
	constexpr static int ALBUM = 3;
	set getSettings();
	~Settings();
public slots:
	void on_confirmButton_clicked();
private:
	Ui::SettingsClass ui;
signals:
	void confirmPressed(set entity);
};
