/**
 * @file AboutPage.cpp
 * @brief 关于页面对话框类的实现
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "AboutPage.h"

/**
 * @brief 构造函数，初始化关于页面
 * @param parent 父窗口指针
 */
AboutPage::AboutPage(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);                                    // 设置UI界面
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));   // 设置窗口图标
    this->setFixedSize(330, 158);                        // 设置固定窗口大小
}
