#pragma once

#include <QDialog>
#include "MusicProperties.h"
#include "ui_AddRuleWidget.h"

class AddRuleWidget final : public QDialog
{
	Q_OBJECT

public:
	explicit AddRuleWidget(QWidget *parent = nullptr);
	QMap<QPair<PROPERTIES::lyricRules, PROPERTIES::ignoreLyricRules>, QString> getRules();
	~AddRuleWidget() override = default;
private:
	Ui::AddRuleWidgetClass ui;
signals:
	void sendRules(QMap<QPair<PROPERTIES::lyricRules, PROPERTIES::ignoreLyricRules>, QString>);
public slots:
	void on_confirmButton_clicked();

};
