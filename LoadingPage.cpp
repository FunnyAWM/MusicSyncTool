#include "LoadingPage.h"
#include "MusicSyncTool.h"

LoadingPage::LoadingPage(QWidget *parent)
	: QWidget(parent) {
	ui.setupUi(this);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}

void LoadingPage::showPage() {
	show();
}

void LoadingPage::stopPage() {
	close();
}
