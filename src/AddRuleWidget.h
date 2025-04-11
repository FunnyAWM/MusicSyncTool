#pragma once


#include "LyricIgnoreRule.h"
#include "ui_AddRuleWidget.h"

class AddRuleWidget final : public QDialog
{
	Q_OBJECT

public:
	explicit AddRuleWidget(QWidget *parent = nullptr);
	LyricIgnoreRule getRules() const;
	~AddRuleWidget() override = default;
private:
	Ui::AddRuleWidgetClass ui;
signals:
	void sendRules(LyricIgnoreRule);
public slots:
	void on_confirmButton_clicked();
};
