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
	
private:
	Ui::LoadingPageClass ui;
    int total;
public slots:
    void showPage();
    void setTotal(int total);
    void setProgress(int value);
    void stopPage();
};

#endif // LOADINGPAGE_H
