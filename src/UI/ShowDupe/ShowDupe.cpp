/**
 * @file ShowDupe.cpp
 * @brief 显示重复项对话框类的实现
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "ShowDupe.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTranslator>

/**
 * @brief 构造函数，初始化显示重复项对话框
 * @param parent 父窗口指针
 */
ShowDupe::ShowDupe(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);                                     // 设置UI界面
	this->setWindowModality(Qt::ApplicationModal);       // 设置为应用程序模态对话框
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));   // 设置窗口图标
}

/**
 * @brief 向列表中添加数据项
 * @param data 要添加的数据字符串
 */
void ShowDupe::add(const QString& data) const { 
    ui.listWidget->addItem(data); 
}
