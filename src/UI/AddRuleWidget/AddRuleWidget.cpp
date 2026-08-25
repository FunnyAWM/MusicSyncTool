#include "AddRuleWidget.h"

#include <QDialog>

#include "../../Core/MusicProperties.h"

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
	getRules().setRulesStr();
	emit sendRules(getRules());
	close();
}
