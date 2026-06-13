/**
 * @file Settings.h
 * @brief 设置界面类定义
 * @details 定义了应用程序设置的用户界面，包括歌词忽略、排序方式、
 *          语言、收藏标签和忽略规则等设置项
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QJsonObject>

#include "ui_Settings.h"
#include "../../Core/SettingEntity.h"
#include "../../Data/LyricIgnoreRule.h"



/**
 * @brief 设置界面类
 * 提供应用程序设置的用户界面，包括歌词忽略、排序方式、语言等设置
 */
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
	Ui::SettingsClass ui;  ///< UI界面对象
	SettingsData entity;            ///< 设置实体对象

signals:
	/**
	 * @brief 确认按钮按下信号
	 * @param entity 设置实体对象
	 */
	void confirmPressed(SettingsData entity);
};

#endif // SETTINGS_H
