/**
 * @file LoadingPage.h
 * @brief 加载页面类定义
 * @details 定义了显示加载进度和随机标题的界面
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include "ui_LoadingPage.h"

/**
 * @brief 加载页面类
 * 显示加载进度和随机标题的界面，用于长时间操作的进度反馈
 */
class LoadingPage final : public QWidget {
    Q_OBJECT

public:
    explicit LoadingPage(QWidget *parent = nullptr);
    
    ~LoadingPage() override = default;
    
    void setTitle(const QString& title) const;
    
    void setRandomTitle() const;

private:
    Ui::LoadingPageClass ui; ///< UI界面对象
    qsizetype total;         ///< 总数量，用于计算进度百分比

public slots:
    void showPage();
    
    void setTotal(qsizetype total_);
    
    void setProgress(qsizetype value) const;
    
    void stopPage();
};

#endif // LOADINGPAGE_H
