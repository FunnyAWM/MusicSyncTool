#pragma once

#include <QWidget>
#include "ui_LoadingPage.h"

class LoadingPage : public QWidget
{
	Q_OBJECT

public:
	LoadingPage(QWidget *parent = nullptr);
	~LoadingPage() = default;
private:
	Ui::LoadingPageClass ui;
};
