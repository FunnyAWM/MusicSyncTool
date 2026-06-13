/**
 * @file AddRuleWidget.h
 * @brief 添加规则对话框类定义
 * @details 定义了用于添加歌词忽略规则的用户界面对话框
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef ADDRULEWIDGET_H
#define ADDRULEWIDGET_H

#include "ui_AddRuleWidget.h"
#include "../../Data/LyricIgnoreRule.h"

/**
 * @brief 添加规则对话框类
 * 用于添加歌词忽略规则的用户界面
 */
class AddRuleWidget final : public QDialog
{
	Q_OBJECT

public:
	explicit AddRuleWidget(QWidget *parent = nullptr);
	
	LyricIgnoreRule getRules() const;
	
	~AddRuleWidget() override = default;

private:
	Ui::AddRuleWidgetClass ui; ///< UI界面对象

signals:
	/**
	 * @brief 发送规则信号
	 * @param rule 要发送的歌词忽略规则
	 */
	void sendRules(LyricIgnoreRule rule);

public slots:
	void on_confirmButton_clicked();
};

#endif // ADDRULEWIDGET_H
