/**
 * @file OperationResult.cpp
 * @brief 操作结果对话框类的实现
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "OperationResult.h"
#include <QDialog>

/**
 * @brief 构造函数，初始化操作结果对话框
 * @param parent 父窗口指针
 */
OperationResult::OperationResult(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);                                     // 设置UI界面
	this->setWindowModality(Qt::ApplicationModal);       // 设置为应用程序模态对话框
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));   // 设置窗口图标
}

/**
 * @brief 设置要显示的结果文本
 * @param text 结果文本内容
 */
void OperationResult::setText(const QString& text) const { 
    ui.textEdit->setText(text); 
}
