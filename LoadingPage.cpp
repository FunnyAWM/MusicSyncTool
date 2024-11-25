#include "LoadingPage.h"

LoadingPage::LoadingPage(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::LoadingPageClass())
{
	ui->setupUi(this);
}

LoadingPage::~LoadingPage()
{
	delete ui;
}
