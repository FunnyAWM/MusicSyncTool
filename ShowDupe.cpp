#include "ShowDupe.h"

ShowDupe::ShowDupe(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

void ShowDupe::add(QString data)
{
	ui.listWidget->addItem(data);
}

ShowDupe::~ShowDupe()
{}
