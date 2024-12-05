#pragma once

#include <QThread>
#include "LoadingPage.h"

class LoadingOperation  : public QThread
{
	Q_OBJECT
	LoadingPage loading;
public:
	LoadingOperation(QObject *parent = nullptr);
	void run() override;
	~LoadingOperation();
};
