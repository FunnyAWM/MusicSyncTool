/**
 * @file LoadingPage.cpp
 * @brief 加载页面类的实现
 * @details 实现加载进度页面的初始化、进度更新和随机标题显示功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "LoadingPage.h"
#include <ctime>
#include "../MusicSyncTool/MSTMainWindow.h"

/**
 * @brief 加载页面构造函数
 * @param parent 父窗口指针
 */
LoadingPage::LoadingPage(QWidget* parent) : QWidget(parent), total(0) {
	ui.setupUi(this); // 设置UI界面
	this->setWindowFlags(Qt::FramelessWindowHint); // 设置无边框窗口
	this->setWindowModality(Qt::ApplicationModal); // 设置为应用程序模态
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico")); // 设置窗口图标
	setRandomTitle(); // 设置随机标题
}

/**
 * @brief 设置加载页面标题
 * @param title 要显示的标题文本
 */
void LoadingPage::setTitle(const QString& title) const {
	ui.label->setText(title);
}

/**
 * @brief 从文件中随机选择并设置标题
 * 从titles.txt文件中随机选择一个标题进行显示
 */
void LoadingPage::setRandomTitle() const {
	srand(static_cast<unsigned>(time(nullptr))); // 初始化随机数种子
	QFile file(":/MusicSyncTool/titles.txt"); // 从Qt资源系统读取标题文件
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		const int totalTitle = stream.readLine().toInt(); // 读取标题总数
		const int titleIndex = rand() % totalTitle; // 随机选择标题索引
		// 跳转到指定的标题行
		for (int i = 0; i < titleIndex; i++) {
			stream.readLine();
		}
		const QString title = stream.readLine(); // 读取标题
		setTitle(title); // 设置标题
		file.close();
	}
}

/**
 * @brief 显示加载页面
 * 重置进度条并设置随机标题后显示页面
 */
void LoadingPage::showPage() {
	ui.progressBar->setValue(0); // 重置进度条为0
	setRandomTitle(); // 设置新的随机标题
	show(); // 显示页面
}

/**
 * @brief 停止加载页面
 * 关闭加载页面
 */
void LoadingPage::stopPage() {
	close();
}

/**
 * @brief 设置进度条的当前进度
 * @param value 当前进度值
 */
void LoadingPage::setProgress(const qsizetype value) const {
	ui.progressBar->setValue(static_cast<int>(std::round(static_cast<double>(value) / static_cast<double>(total) * 100.0))); // 计算并设置百分比进度
}

/**
 * @brief 设置总数量
 * @param total_ 总数量，用于计算进度百分比
 */
void LoadingPage::setTotal(const qsizetype total_) {
	this->total = total_;
}
