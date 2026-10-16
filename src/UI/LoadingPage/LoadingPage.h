#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include "ui_LoadingPage.h"

class LoadingPage final : public QWidget {
    Q_OBJECT

public:
    explicit LoadingPage(QWidget *parent = nullptr);
    
    ~LoadingPage() override = default;
    
    void setTitle(const QString& title) const;
    
    void setRandomTitle() const;

private:
    Ui::LoadingPageClass ui;
    int total;

public slots:
    void showPage();
    
    // 进度计数统一用 int：跨线程排队信号的可注册元类型，避免 qsizetype 在 moc 侧解析失败
    void setTotal(int total_);
    
    void setProgress(int value) const;
    
    void stopPage();
};

#endif // LOADINGPAGE_H
