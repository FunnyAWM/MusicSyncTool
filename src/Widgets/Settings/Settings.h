#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTranslator>
#include <QWidget>

#include "../../LyricIgnoreRule.h"
#include "../../MusicProperties.h"
#include "../../SettingEntity.h"
#include "ui_Settings.h"

using namespace PROPERTIES;

/**
 * @brief 设置界面类
 * 提供应用程序设置的用户界面，包括歌词忽略、排序方式、语言等设置
 */
class Settings final : public QWidget {
	Q_OBJECT

public:
	/**
	 * @brief 构造函数
	 * @param parent 父窗口指针，默认为nullptr
	 */
	explicit Settings(QWidget* parent = nullptr);
	
	/**
	 * @brief 获取当前设置
	 * @return 返回包含所有设置的实体对象
	 */
	[[nodiscard]] set getSettings();
	
	/**
	 * @brief 设置歌词忽略选项到UI
	 * @param ignoreLyric 是否忽略歌词
	 */
	void setIgnoreLyricToUI(bool ignoreLyric);
	
	/**
	 * @brief 设置排序方式到UI
	 * @param sortBy 排序方式
	 */
	void setSortByToUI(short sortBy);
	
	/**
	 * @brief 设置语言到UI
	 * @param language 语言设置
	 */
	void setLanguageToUI(QString language);
	
	/**
	 * @brief 设置收藏标签到UI
	 * @param favoriteTag 收藏标签
	 */
	void setFavoriteTagToUI(const QString& favoriteTag);
	
	/**
	 * @brief 设置排序顺序到UI
	 * @param orderBy 排序顺序
	 */
	void setOrderByToUI(short orderBy);
	
	/**
	 * @brief 设置忽略规则列表到UI
	 * @param rules 忽略规则列表
	 */
	void setIgnoreRulesToUI(QList<LyricIgnoreRule>& rules);

	void setRecursiveScanToUI(const bool recursiveScan);
	/**
	 * @brief 析构函数
	 */
	~Settings() override = default;

public slots:
	/**
	 * @brief 确认按钮点击事件槽函数
	 */
	void on_confirmButton_clicked();
	
	/**
	 * @brief 添加规则按钮点击事件槽函数
	 */
	void on_addRule_clicked() const;
	
	/**
	 * @brief 删除选中规则按钮点击事件槽函数
	 */
	void on_deleteSelectedRule_clicked();
	
	/**
	 * @brief 添加规则槽函数
	 * @param rule 要添加的歌词忽略规则
	 */
	void addRule(const LyricIgnoreRule& rule);

private:
	Ui::SettingsClass ui;  // UI界面对象
	set entity;            // 设置实体对象

signals:
	/**
	 * @brief 确认按钮按下信号
	 * @param entity 设置实体对象
	 */
	void confirmPressed(set entity);
};

#endif // SETTINGS_H
