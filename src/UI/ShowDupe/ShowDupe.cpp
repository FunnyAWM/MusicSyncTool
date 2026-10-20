#include "ShowDupe.h"
#include <QFile>
#include <QJsonObject>

ShowDupe::ShowDupe(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);
	this->setWindowModality(Qt::ApplicationModal);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}

void ShowDupe::add(const QString& data) const { 
    ui.listWidget->addItem(data); 
}
