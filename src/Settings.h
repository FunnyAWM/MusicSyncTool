#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
	[[nodiscard]] set getSettings() const;
	void setIgnoreLyricToUI(bool);
	void setSortByToUI(short);
	void setLanguageToUI(QString);
	void setFavoriteTagToUI(const QString&);
	void setOrderByToUI(short);
	void setIgnoreRulesToUI(const QList<LyricIgnoreRuleSingleton>&) const;
	~Settings() override = default;

public slots:
	void on_confirmButton_clicked();
	void on_addRule_clicked();
	void addRule(LyricIgnoreRuleSingleton);
private:
	Ui::SettingsClass ui;
	set entity;
signals:
	void confirmPressed(set entity);
};

#endif // SETTINGS_H
