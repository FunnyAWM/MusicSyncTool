#pragma once

#include <QDialog>

#include "LyricIgnoreRuleSingleton.h"
#include "MusicProperties.h"
#include "ui_AddRuleWidget.h"

class AddRuleWidget final : public QDialog
{
	Q_OBJECT

public:
	explicit AddRuleWidget(QWidget *parent = nullptr);
	LyricIgnoreRuleSingleton getRules() const;
	~AddRuleWidget() override = default;
private:
	Ui::AddRuleWidgetClass ui;
signals:
	void sendRules(LyricIgnoreRuleSingleton);
public slots:
	void on_confirmButton_clicked();
};
