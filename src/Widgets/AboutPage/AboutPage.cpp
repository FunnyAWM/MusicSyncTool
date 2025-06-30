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
