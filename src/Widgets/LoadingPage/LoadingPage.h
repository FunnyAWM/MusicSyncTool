#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include "ui_LoadingPage.h"

/**
 * @brief 加载页面类
 * 显示加载进度和随机标题的界面
 */
class LoadingPage final : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针，默认为nullptr
     */
    explicit LoadingPage(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~LoadingPage() override = default;
    
    /**
     * @brief 设置加载页面标题
     * @param title 要显示的标题文本
     */
    void setTitle(const QString& title) const;
    
    /**
     * @brief 设置随机标题（从文件中随机选择）
     */
    void setRandomTitle() const;

private:
    Ui::LoadingPageClass ui; // UI界面对象
    qsizetype total;         // 总数量，用于计算进度百分比

public slots:
    /**
     * @brief 显示加载页面
     */
    void showPage();
    
    /**
     * @brief 设置总数量
     * @param total 总数量
     */
    void setTotal(qsizetype total);
    
    /**
     * @brief 设置当前进度
     * @param value 当前进度值
     */
    void setProgress(qsizetype value) const;
    
    /**
     * @brief 停止并关闭加载页面
     */
    void stopPage();
};

#endif // LOADINGPAGE_H
