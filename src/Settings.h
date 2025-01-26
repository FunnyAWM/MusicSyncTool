#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTranslator>
#include <QWidget>

#include "LyricIgnoreRuleSingleton.h"
#include "MusicProperties.h"
#include "SettingEntity.h"
#include "ui_Settings.h"

using namespace PROPERTIES;

class Settings final : public QWidget {
	Q_OBJECT

public:
	explicit Settings(QWidget* parent = nullptr);
	[[nodiscard]] set getSettings();
	void setIgnoreLyricToUI(bool);
	void setSortByToUI(short);
	void setLanguageToUI(QString);
	void setFavoriteTagToUI(const QString&);
	void setOrderByToUI(short);
	void setIgnoreRulesToUI(QList<LyricIgnoreRuleSingleton>&);
	~Settings() override = default;

public slots:
	void on_confirmButton_clicked();
	void on_addRule_clicked();
	void on_deleteSelectedRule_clicked();
	void addRule(const LyricIgnoreRuleSingleton&);
private:
	Ui::SettingsClass ui;
	set entity;
signals:
	void confirmPressed(set entity);
};

#endif // SETTINGS_H
