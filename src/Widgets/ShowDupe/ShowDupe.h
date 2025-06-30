#ifndef SHOWDUPE_H
#define SHOWDUPE_H

#include <QDialog>
#include "ui_ShowDupe.h"

/**
 * @brief 显示重复项对话框类
 * 用于显示重复音乐文件的对话框
 */
class ShowDupe : public QDialog {
	Q_OBJECT

public:
	/**
	 * @brief 构造函数
	 * @param parent 父窗口指针，默认为nullptr
	 */
	explicit ShowDupe(QWidget* parent = nullptr);
	
	/**
	 * @brief 添加数据到列表
	 * @param data 要添加的数据字符串
	 */
	void add(const QString& data) const;
	
	/**
	 * @brief 析构函数
	 */
	~ShowDupe() override = default;

private:
	Ui::ShowDupeClass ui; // UI界面对象
};

#endif // SHOWDUPE_H
