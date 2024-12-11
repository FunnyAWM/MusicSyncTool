#include "CopyResult.h"

CopyResult::CopyResult(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}

void CopyResult::setText(QString text)
{
	ui.textEdit->setText(text);
}

CopyResult::~CopyResult()
{}
