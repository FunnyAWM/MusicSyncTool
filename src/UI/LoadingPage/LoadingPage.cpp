#include "LoadingPage.h"
#include <algorithm>
#include <cmath>
#include <QRandomGenerator>
#include "../MusicSyncTool/MSTMainWindow.h"
#include "../../Services/Logger.h"

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
	QFile file(":/MusicSyncTool/titles.txt");
	if (!file.open(QIODevice::ReadOnly)) {
		Logger::Warn("Failed to open titles.txt, keeping default loading title");
		return;
	}
	QTextStream stream(&file);
	bool titleCountValid = false;
	const int totalTitle = stream.readLine().toInt(&titleCountValid);
	if (!titleCountValid || totalTitle <= 0) {
		// 首行为非法数量时不做取模，避免除零
		Logger::Warn("Invalid title count in titles.txt, keeping default loading title");
		file.close();
		return;
	}
	const int titleIndex = QRandomGenerator::global()->bounded(totalTitle);
	for (int i = 0; i < titleIndex; i++) {
		stream.readLine();
	}
	setTitle(stream.readLine());
	file.close();
}

void LoadingPage::showPage() {
	ui.progressBar->setValue(0);
	setRandomTitle();
	show();
}

void LoadingPage::stopPage() {
	close();
}

void LoadingPage::setProgress(const int value) const {
	if (total <= 0) {
		// 总数未知或为 0 时不做除法，避免出现 NaN 进度
		ui.progressBar->setValue(0);
		return;
	}
	const double percent = static_cast<double>(value) / static_cast<double>(total) * 100.0;
	ui.progressBar->setValue(std::clamp(static_cast<int>(std::round(percent)), 0, 100));
}

void LoadingPage::setTotal(const int total_) {
	this->total = total_;
}
