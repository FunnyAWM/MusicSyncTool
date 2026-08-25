#ifndef ADDRULEWIDGET_H
#define ADDRULEWIDGET_H

#include "ui_AddRuleWidget.h"
#include "../../Data/LyricIgnoreRule.h"

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
	void sendRules(LyricIgnoreRule rule);

public slots:
	void on_confirmButton_clicked();
};

#endif // ADDRULEWIDGET_H
