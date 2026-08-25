#include "LoadingPage.h"
#include <ctime>
#include "../MusicSyncTool/MSTMainWindow.h"

LoadingPage::LoadingPage(QWidget* parent) : QWidget(parent), total(0) {
	ui.setupUi(this);
	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setWindowModality(Qt::ApplicationModal);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
	setRandomTitle();
}

void LoadingPage::setTitle(const QString& title) const {
	ui.label->setText(title);
}

void LoadingPage::setRandomTitle() const {
	srand(static_cast<unsigned>(time(nullptr)));
	QFile file(":/MusicSyncTool/titles.txt");
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		const int totalTitle = stream.readLine().toInt();
		const int titleIndex = rand() % totalTitle;
		for (int i = 0; i < titleIndex; i++) {
			stream.readLine();
		}
		const QString title = stream.readLine();
		setTitle(title);
		file.close();
	}
}

void LoadingPage::showPage() {
	ui.progressBar->setValue(0);
	setRandomTitle();
	show();
}

void LoadingPage::stopPage() {
	close();
}

void LoadingPage::setProgress(const qsizetype value) const {
	ui.progressBar->setValue(static_cast<int>(std::round(static_cast<double>(value) / static_cast<double>(total) * 100.0)));
}

void LoadingPage::setTotal(const qsizetype total_) {
	this->total = total_;
}
