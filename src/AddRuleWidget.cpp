#include "AddRuleWidget.h"

AddRuleWidget::AddRuleWidget(QWidget* parent)
	: QDialog(parent) {
	ui.setupUi(this);
	this->setWindowModality(Qt::ApplicationModal);
}

LyricIgnoreRuleSingleton AddRuleWidget::getRules() const {
	RuleField lyricRules;
	RuleType ignoreLyricRules;
	const QString rule = ui.valueEdit->text();
	switch (ui.typeComboBox->currentIndex()) {
	case 0:
		lyricRules = RuleField::TITLE;
		break;
	case 1:
		lyricRules = RuleField::ARTIST;
		break;
	case 2:
		lyricRules = RuleField::ALBUM;
		break;
	}
	switch (ui.ruleComboBox->currentIndex()) {
	case 0:
		ignoreLyricRules = RuleType::INCLUDES;
		break;
	case 1:
		ignoreLyricRules = RuleType::EXCLUDES;
		break;
	}
	return LyricIgnoreRuleSingleton(ignoreLyricRules, lyricRules, rule);
}

void AddRuleWidget::on_confirmButton_clicked() {
	getRules().setRulesStr();
	emit sendRules(getRules());
	close();
}


