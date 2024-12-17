#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include <QWidget>
#include "ui_LoadingPage.h"

class LoadingPage : public QWidget
{
	Q_OBJECT

public:
	LoadingPage(QWidget *parent = nullptr);
	~LoadingPage() = default;
	void showPage();
	void stopPage();
private:
	Ui::LoadingPageClass ui;
};

#endif // LOADINGPAGE_H
