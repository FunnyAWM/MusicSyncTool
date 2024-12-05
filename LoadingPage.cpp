#include "LoadingPage.h"
#include "MusicSyncTool.h"

LoadingPage::LoadingPage(QWidget *parent)
	: QWidget(parent) {
	ui.setupUi(this);
}

void LoadingPage::showPage() {
	show();
}

void LoadingPage::stopPage() {
	close();
}
