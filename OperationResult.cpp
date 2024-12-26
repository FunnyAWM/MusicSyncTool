#include "OperationResult.h"

OperationResult::OperationResult(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}

void OperationResult::setText(QString text) { ui.textEdit->setText(text); }
