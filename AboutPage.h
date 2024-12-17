#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QDialog>
#include "ui_AboutPage.h"

class AboutPage : public QDialog
{
	Q_OBJECT

public:
	AboutPage(QWidget *parent = nullptr);
	~AboutPage() = default;

private:
	Ui::AboutPageClass ui;
};

#endif // ABOUTPAGE_H
