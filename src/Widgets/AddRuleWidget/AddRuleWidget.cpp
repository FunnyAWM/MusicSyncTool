#include "AddRuleWidget.h"

#include <QDialog>

#include "../../MusicProperties.h"

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
	RuleField lyricRules;      // 规则字段（标题/艺术家/专辑）
	RuleType ignoreLyricRules; // 规则类型（包含/排除）
	const QString rule = ui.valueEdit->text(); // 规则值
	
	// 根据类型下拉框选择设置规则字段
	switch (ui.typeComboBox->currentIndex()) {
	case 0:
		lyricRules = RuleField::TITLE;  // 标题
		break;
	case 1:
		lyricRules = RuleField::ARTIST; // 艺术家
		break;
	case 2:
		lyricRules = RuleField::ALBUM;  // 专辑
		break;
	default:
	    break;
	}
	
	// 根据规则下拉框选择设置规则类型
	switch (ui.ruleComboBox->currentIndex()) {
	case 0:
		ignoreLyricRules = RuleType::INCLUDES; // 包含
		break;
	case 1:
		ignoreLyricRules = RuleType::EXCLUDES; // 排除
		break;
	default:
	    break;
	}
	return LyricIgnoreRule(ignoreLyricRules, lyricRules, rule);
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


