#ifndef OPERATIONRESULT_H
#define OPERATIONRESULT_H

#include "ui_OperationResult.h"

/**
 * @brief 操作结果对话框类
 * 用于显示操作执行结果的对话框
 */
class OperationResult final : public QDialog {
	Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针，默认为nullptr
     */
    explicit OperationResult(QWidget* parent = nullptr);
	
	/**
	 * @brief 设置结果文本
	 * @param text 要显示的结果文本
	 */
	void setText(const QString& text) const;
	
	/**
	 * @brief 析构函数
	 */
	~OperationResult() override = default;

private:
	Ui::OperationResultClass ui; // UI界面对象
};

#endif // OPERATIONRESULT_H
