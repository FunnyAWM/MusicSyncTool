#include "AboutPage.h"

AboutPage::AboutPage(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
	this->setFixedSize(330,158);
}
