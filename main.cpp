﻿#include "MusicSyncTool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MusicSyncTool w;
    w.show();
    return a.exec();
}
