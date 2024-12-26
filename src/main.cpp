#include <QTranslator>
#include <QtWidgets/QApplication>
#include "MusicSyncTool.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MusicSyncTool w;
    w.show();
    return a.exec();
}
