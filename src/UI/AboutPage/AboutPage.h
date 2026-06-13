/**
 * @file AboutPage.h
 * @brief 关于页面对话框类定义
 * @details 定义了显示应用程序版本信息和相关信息的对话框
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include "ui_AboutPage.h"

/**
 * @brief 关于页面对话框类
 * 显示应用程序的版本信息和相关信息
 */
class AboutPage final : public QDialog {
	Q_OBJECT

public:
	explicit AboutPage(QWidget* parent = nullptr);
	
	~AboutPage() override = default;

private:
	Ui::AboutPageClass ui; ///< UI界面对象
};

#endif // ABOUTPAGE_H
