#include "LoadingOperation.h"

LoadingOperation::LoadingOperation(QObject *parent)
	: QThread(parent)
{}

void LoadingOperation::run()
{
	loading.show();
}

LoadingOperation::~LoadingOperation()
{
	loading.close();
}
