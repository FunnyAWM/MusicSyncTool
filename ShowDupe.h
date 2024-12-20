#ifndef SHOWDUPE_H
#define SHOWDUPE_H

#include <QDialog>
#include "ui_ShowDupe.h"

class ShowDupe : public QDialog {
    Q_OBJECT

public:
    ShowDupe(QWidget *parent = nullptr);
    void add(QString);
    void loadLanguage();
    ~ShowDupe();

private:
    Ui::ShowDupeClass ui;
};

#endif // SHOWDUPE_H
