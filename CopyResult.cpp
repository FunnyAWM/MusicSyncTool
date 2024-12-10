#include "CopyResult.h"

CopyResult::CopyResult(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

void CopyResult::setText(QString text)
{
	ui.textEdit->setText(text);
}

CopyResult::~CopyResult()
{}
