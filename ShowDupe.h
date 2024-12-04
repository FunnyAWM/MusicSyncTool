#pragma once

#include <QDialog>
#include "ui_ShowDupe.h"

class ShowDupe : public QDialog
{
	Q_OBJECT

public:
	ShowDupe(QWidget *parent = nullptr);
	void add(QString);
	~ShowDupe();

private:
	Ui::ShowDupeClass ui;
};
