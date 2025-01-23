#include "OperationResult.h"

OperationResult::OperationResult(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);
	this->setWindowModality(Qt::ApplicationModal);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}

void OperationResult::setText(const QString& text) const { ui.textEdit->setText(text); }
