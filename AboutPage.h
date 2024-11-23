#pragma once

#include <QDialog>
#include "ui_AboutPage.h"

class AboutPage : public QDialog
{
	Q_OBJECT

public:
	AboutPage(QWidget *parent = nullptr);
	~AboutPage();

private:
	Ui::AboutPageClass ui;
};
