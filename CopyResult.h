#pragma once

#include <QDialog>
#include "ui_CopyResult.h"

class CopyResult : public QDialog
{
	Q_OBJECT

public:
	CopyResult(QWidget *parent = nullptr);
	void setText(QString text);
	~CopyResult();

private:
	Ui::CopyResultClass ui;
};
