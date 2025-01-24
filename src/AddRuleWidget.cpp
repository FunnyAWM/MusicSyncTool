#include "AddRuleWidget.h"

AddRuleWidget::AddRuleWidget(QWidget* parent)
	: QDialog(parent) {
	ui.setupUi(this);
	this->setWindowModality(Qt::ApplicationModal);
}

LyricIgnoreRuleSingleton AddRuleWidget::getRules() const {
	lyricRules lyricRules;
	ignoreLyricRules ignoreLyricRules;
	QString rule;
	rule = ui.valueEdit->text();
	switch (ui.ruleComboBox->currentIndex()) {
	case 0:
		lyricRules = lyricRules::NAME;
		break;
	case 1:
		lyricRules = lyricRules::ARTIST;
		break;
	case 2:
		lyricRules = lyricRules::ALBUM;
		break;
	}
	switch (ui.typeComboBox->currentIndex()) {
	case 0:
		ignoreLyricRules = ignoreLyricRules::INCLUDES;
		break;
	case 1:
		ignoreLyricRules = ignoreLyricRules::EXCLUDES;
		break;
	}
	return LyricIgnoreRuleSingleton(ignoreLyricRules, lyricRules, rule);
}

void AddRuleWidget::on_confirmButton_clicked() {
	emit sendRules(getRules());
	close();
}


