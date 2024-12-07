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
	};
	Settings(QWidget *parent = nullptr);
	set getSettings();
	~Settings();
public slots:
	void on_confirmButton_clicked();
private:
	Ui::SettingsClass ui;
signals:
	void confirmPressed(set entity);
};
