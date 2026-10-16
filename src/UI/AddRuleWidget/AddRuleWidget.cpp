#include "AddRuleWidget.h"

#include <QDialog>

#include "../../Core/MusicProperties.h"

// LyricIgnoreRule 不再继承 QObject，原先经它传递到本翻译单元的 PROPERTIES 名字改为显式引入
using PROPERTIES::RuleField;
using PROPERTIES::RuleType;

AddRuleWidget::AddRuleWidget(QWidget* parent)
	: QDialog(parent) {
	ui.setupUi(this);
	this->setWindowModality(Qt::ApplicationModal);
}

LyricIgnoreRule AddRuleWidget::getRules() const {
	RuleField selectedRuleField;
	RuleType selectedRuleType;
	const QString rule = ui.valueEdit->text();
	
	switch (ui.typeComboBox->currentIndex()) {
	case 0:
		selectedRuleField = RuleField::TITLE;
		break;
	case 1:
		selectedRuleField = RuleField::ARTIST;
		break;
	case 2:
		selectedRuleField = RuleField::ALBUM;
		break;
	default:
		selectedRuleField = RuleField::NONE;
	    break;
	}
	
	switch (ui.ruleComboBox->currentIndex()) {
	case 0:
		selectedRuleType = RuleType::INCLUDES;
		break;
	case 1:
		selectedRuleType = RuleType::EXCLUDES;
		break;
	default:
		selectedRuleType = RuleType::NONE;
	    break;
	}
	return LyricIgnoreRule(selectedRuleType, selectedRuleField, rule);
}

void AddRuleWidget::on_confirmButton_clicked() {
	// 规则字符串由接收方（Settings::setIgnoreRulesToUI）统一生成，
	// 此处对返回值调用 setRulesStr 只是作用于临时对象，不会生效
	emit sendRules(getRules());
	close();
}
