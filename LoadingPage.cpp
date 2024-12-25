#include "LoadingPage.h"
#include "MusicSyncTool.h"

LoadingPage::LoadingPage(QWidget *parent) : QWidget(parent), total(0) {
    ui.setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}

void LoadingPage::setTitle(QString title) { this->setWindowTitle(title); }

void LoadingPage::showPage() { 
    ui.progressBar->setValue(0);
    show();
}

void LoadingPage::stopPage() { close(); }

void LoadingPage::setProgress(int value) { ui.progressBar->setValue((double(value) / total) * 100); }

void LoadingPage::setTotal(int total) { this->total = total; }
