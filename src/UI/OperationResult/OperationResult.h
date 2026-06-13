/**
 * @file OperationResult.h
 * @brief 操作结果对话框类定义
 * @details 定义了用于显示操作执行结果的对话框
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

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
    explicit OperationResult(QWidget* parent = nullptr);
	
	void setText(const QString& text) const;
	
	~OperationResult() override = default;

private:
	Ui::OperationResultClass ui; ///< UI界面对象
};

#endif // OPERATIONRESULT_H
