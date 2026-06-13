/**
 * @file MSTErrorReporter.h
 * @brief 错误报告与操作结果展示类定义
 * @details 封装错误对话框弹出、错误列表管理和操作结果展示逻辑，
 *          将错误处理职责从主窗口中分离出来
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTERRORREPORTER_H
#define MSTERRORREPORTER_H

#include <QObject>
#include <QStringList>

#include "../../Core/MusicProperties.h"

class QWidget;
class MSTTableManager;

using PROPERTIES::FileErrorType;
using PROPERTIES::LoadErrorType;
using PROPERTIES::OperationType;
using PROPERTIES::PathType;
using PROPERTIES::AppErrorType;

/**
 * @brief 错误报告器
 * @details 负责应用程序的错误处理和操作结果展示，包括：
 *          - 根据错误类型弹出相应的错误/提示对话框
 *          - 管理文件操作和加载过程中的错误信息列表
 *          - 展示操作结果对话框并在完成后刷新表格
 */
class MSTErrorReporter : public QObject {
	Q_OBJECT

public:
	explicit MSTErrorReporter(QWidget* parentWidget, MSTTableManager* tableManager,
	                         QObject* parent = nullptr);

	void popError(AppErrorType type) const;

	void addToErrorList(const QString& file, FileErrorType error);

	void addToErrorList(const QString& file, LoadErrorType error);

	void showOperationResult(OperationType type);

signals:
	/**
	 * @brief 请求刷新音乐列表
	 * @param path 路径类型
	 * @param page 页码
	 */
	void requestRefreshMusic(PathType path, unsigned short page);

private:
	QWidget* parentWidget;             ///< 父窗口部件
	MSTTableManager* tableManager;     ///< 表格管理器指针
	QStringList errorList;             ///< 错误信息列表
};

#endif // MSTERRORREPORTER_H
