#include "ShowDupe.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTranslator>

ShowDupe::ShowDupe(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}

void ShowDupe::add(QString data) { ui.listWidget->addItem(data); }

ShowDupe::~ShowDupe() {}
