#pragma once

#include <QDialog>
#include "ui_LoadingPage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoadingPageClass; };
QT_END_NAMESPACE

class LoadingPage : public QDialog
{
	Q_OBJECT

public:
	LoadingPage(QWidget *parent = nullptr);
	~LoadingPage();
	void setProgress(int value);

private:
	Ui::LoadingPageClass *ui;
};
