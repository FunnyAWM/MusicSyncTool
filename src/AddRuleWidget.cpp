#include "AddRuleWidget.h"

AddRuleWidget::AddRuleWidget(QWidget* parent)
	: QDialog(parent) {
	ui.setupUi(this);
}

QMap<QPair<PROPERTIES::lyricRules, PROPERTIES::ignoreLyricRules>, QString> AddRuleWidget::getRules() {
	PROPERTIES::lyricRules lyricRules;
	PROPERTIES::ignoreLyricRules ignoreLyricRules;
	QString rule;
	rule = ui.valueEdit->text();
	switch (ui.ruleComboBox->currentIndex()) {
	case 0:
		lyricRules = PROPERTIES::lyricRules::NAME;
		break;
	case 1:
		lyricRules = PROPERTIES::lyricRules::ARTIST;
		break;
	case 2:
		lyricRules = PROPERTIES::lyricRules::ALBUM;
		break;
	}
	switch (ui.typeComboBox->currentIndex()) {
	case 0:
		ignoreLyricRules = PROPERTIES::ignoreLyricRules::INCLUDES;
		break;
	case 1:
		ignoreLyricRules = PROPERTIES::ignoreLyricRules::EXCLUDES;
		break;
	}
	return QMap<QPair<PROPERTIES::lyricRules, PROPERTIES::ignoreLyricRules>, QString>(
		{ {QPair<PROPERTIES::lyricRules, PROPERTIES::ignoreLyricRules>(lyricRules, ignoreLyricRules), rule} });
}

void AddRuleWidget::on_confirmButton_clicked() {
	emit sendRules(getRules());
	close();
}


