﻿#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include "ui_LoadingPage.h"

class LoadingPage final : public QWidget {
    Q_OBJECT

public:
    explicit LoadingPage(QWidget *parent = nullptr);
    ~LoadingPage() override = default;

private:
    Ui::LoadingPageClass ui;
    qsizetype total;
public slots:
    void showPage();
    void setTotal(qsizetype total);
    void setProgress(qsizetype value) const;
    void stopPage();
};

#endif // LOADINGPAGE_H
