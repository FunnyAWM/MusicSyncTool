#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QJsonObject>

#include "ui_Settings.h"
#include "../../Core/SettingEntity.h"
#include "../../Data/LyricIgnoreRule.h"



class Settings final : public QWidget {
	Q_OBJECT

public:
	explicit Settings(const SettingsData& entity, QWidget* parent = nullptr);
	
	~Settings() override = default;

	[[nodiscard]] SettingsData getSettings();
	
	void setIgnoreLyricToUI(bool ignoreLyric);
	
	void setSortByToUI(short sortBy);
	
	void setOrderByToUI(short orderBy);
	
	void setLanguageToUI(QString language);
	
	void setFavoriteTagToUI(const QString& favoriteTag);
	
	void setRecursiveScanToUI(bool recursiveScan);

	void setIgnoreRulesToUI(QList<LyricIgnoreRule>& rules);

public slots:
	void on_confirmButton_clicked();
	
	void on_addRule_clicked() const;
	
	void on_deleteSelectedRule_clicked();
	
	void addRule(const LyricIgnoreRule& rule);

private:
	Ui::SettingsClass ui;
	SettingsData entity;

signals:
	/**
	 * @brief 确认按钮按下信号
	 * @param entity 设置实体对象
	 */
	void confirmPressed(SettingsData entity);
};

#endif // SETTINGS_H
