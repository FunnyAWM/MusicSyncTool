#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include "ui_LoadingPage.h"

class LoadingPage : public QWidget {
    Q_OBJECT

public:
    explicit LoadingPage(QWidget *parent = nullptr);
    void setTitle(const QString&);
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
