/**
 * @file AddRuleWidget.cpp
 * @brief 添加规则对话框类的实现
 * @details 实现添加歌词忽略规则对话框的界面初始化和规则获取功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "AddRuleWidget.h"

#include <QDialog>

#include "../../Core/MusicProperties.h"

/**
 * @brief 构造函数，初始化添加规则对话框
 * @param parent 父窗口指针
 */
AddRuleWidget::AddRuleWidget(QWidget* parent)
	: QDialog(parent) {
	ui.setupUi(this);                                         // 设置UI界面
	this->setWindowModality(Qt::ApplicationModal);           // 设置为应用程序模态对话框
}

/**
 * @brief 获取用户设置的规则
 * @return 返回根据用户选择创建的歌词忽略规则对象
 */
LyricIgnoreRule AddRuleWidget::getRules() const {
	RuleField selectedRuleField;      // 规则字段（标题/艺术家/专辑）
	RuleType selectedRuleType; // 规则类型（包含/排除）
	const QString rule = ui.valueEdit->text(); // 规则值
	
	// 根据类型下拉框选择设置规则字段
	switch (ui.typeComboBox->currentIndex()) {
	case 0:
		selectedRuleField = RuleField::TITLE;  // 标题
		break;
	case 1:
		selectedRuleField = RuleField::ARTIST; // 艺术家
		break;
	case 2:
		selectedRuleField = RuleField::ALBUM;  // 专辑
		break;
	default:
		selectedRuleField = RuleField::NONE;
	    break;
	}
	
	// 根据规则下拉框选择设置规则类型
	switch (ui.ruleComboBox->currentIndex()) {
	case 0:
		selectedRuleType = RuleType::INCLUDES; // 包含
		break;
	case 1:
		selectedRuleType = RuleType::EXCLUDES; // 排除
		break;
	default:
		selectedRuleType = RuleType::NONE;
	    break;
	}
	return LyricIgnoreRule(selectedRuleType, selectedRuleField, rule);
}

/**
 * @brief 确认按钮点击事件处理函数
 * 获取规则并发送信号，然后关闭对话框
 */
void AddRuleWidget::on_confirmButton_clicked() {
	getRules().setRulesStr();          // 设置规则字符串
	emit sendRules(getRules());        // 发送规则信号
	close();                           // 关闭对话框
}
