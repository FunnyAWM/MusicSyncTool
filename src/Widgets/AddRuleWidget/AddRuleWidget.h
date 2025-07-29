#pragma once

#include "../../LyricIgnoreRule.h"
#include "ui_AddRuleWidget.h"

/**
 * @brief 添加规则对话框类
 * 用于添加歌词忽略规则的用户界面
 */
class AddRuleWidget final : public QDialog
{
	Q_OBJECT

public:
	/**
	 * @brief 构造函数
	 * @param parent 父窗口指针，默认为nullptr
	 */
	explicit AddRuleWidget(QWidget *parent = nullptr);
	
	/**
	 * @brief 获取用户设置的规则
	 * @return 返回配置的歌词忽略规则对象
	 */
	LyricIgnoreRule getRules() const;
	
	/**
	 * @brief 析构函数
	 */
	~AddRuleWidget() override = default;

private:
	Ui::AddRuleWidgetClass ui; // UI界面对象

signals:
	/**
	 * @brief 发送规则信号
	 * @param rule 要发送的歌词忽略规则
	 */
	void sendRules(LyricIgnoreRule rule);

public slots:
	/**
	 * @brief 确认按钮点击事件槽函数
	 */
	void on_confirmButton_clicked();
};
