/**
 * @file MSTErrorReporter.cpp
 * @brief 错误报告与操作结果展示类的实现
 * @details 实现错误对话框弹出、错误列表管理和操作结果展示
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTErrorReporter.h"

#include <QMessageBox>
#include <QWidget>

#include "MSTTableManager.h"
#include "../../Core/SettingEntity.h"
#include "../OperationResult/OperationResult.h"

/**
 * @brief 构造函数
 */
MSTErrorReporter::MSTErrorReporter(QWidget* parentWidget, MSTTableManager* tableManager, QObject* parent)
	: QObject(parent), parentWidget(parentWidget), tableManager(tableManager) {
}

/**
 * @brief 根据错误类型弹出错误对话框
 */
void MSTErrorReporter::popError(const AppErrorType type) const {
	switch (type) {
	case AppErrorType::NO_AUDIO:
		QMessageBox::critical(parentWidget, tr("错误"), tr("没有选定音频！（提示：可以通过双击表格中的歌曲来预览）"));
		break;
	case AppErrorType::NO_PATH:
	case AppErrorType::DUPLICATE_FILE:
		QMessageBox::critical(parentWidget, tr("错误"), tr("没有选定路径！"));
		break;
	case AppErrorType::NO_DEST_PATH:
		QMessageBox::critical(parentWidget, tr("错误"), tr("目标文件夹尚未打开，请先选择目标路径！"));
		break;
	case AppErrorType::NO_FAV_TAG:
		QMessageBox::critical(parentWidget, tr("错误"), tr("没有设置喜爱标签！请在设置中指定！"));
		break;
	case AppErrorType::NO_FILE:
		QMessageBox::critical(parentWidget, tr("错误"), tr("没有选定文件！"));
		break;
	case AppErrorType::FIRST:
		QMessageBox::information(parentWidget, tr("提示"), tr("已经是第一页了！"));
		break;
	case AppErrorType::LAST:
		QMessageBox::information(parentWidget, tr("提示"), tr("已经是最后一页了！"));
		break;
	case AppErrorType::RUNNING:
		QMessageBox::critical(parentWidget, tr("错误"), tr("程序已在运行！"));
		break;
	case AppErrorType::NO_LANGUAGE:
		QMessageBox::critical(parentWidget, tr("错误"), tr("找不到程序语言配置文件，程序即将退出！"));
		break;
	case AppErrorType::DB_ERROR:
		QMessageBox::critical(parentWidget, tr("错误"), tr("操作数据库中数据时出现严重错误，程序即将退出！"));
		break;
	default:
		break;
	}
}

/**
 * @brief 添加文件操作错误到错误列表
 */
void MSTErrorReporter::addToErrorList(const QString& file, const FileErrorType error) {
	switch (error) {
	case FileErrorType::DUPLICATE:
		errorList.append(tr("复制") + file + tr("失败：文件已存在"));
		break;
	case FileErrorType::LYRIC_NOT_FOUND:
		errorList.append(tr("复制") + file + tr("失败：找不到歌词文件"));
		break;
	case FileErrorType::DISKFULL:
		errorList.append(tr("复制") + file + tr("失败：磁盘已满"));
		break;
	}
}

/**
 * @brief 添加加载错误到错误列表
 */
void MSTErrorReporter::addToErrorList(const QString& file, const LoadErrorType error) {
	switch (error) {
	case LoadErrorType::FILE_NOT_SCANNABLE:
		errorList.append(tr("加载") + file + tr("失败：文件不可扫描"));
		break;
	case LoadErrorType::TAG_READ_ERROR:
		errorList.append(tr("加载") + file + tr("失败：标签错误"));
		break;
	}
}

/**
 * @brief 显示操作结果对话框
 */
void MSTErrorReporter::showOperationResult(const OperationType type) {
	const auto resultDialog = new OperationResult();
	switch (type) {
	case OperationType::COPY:
		resultDialog->setWindowTitle(tr("复制结果"));
		break;
	case OperationType::LOAD:
		resultDialog->setWindowTitle(tr("加载结果"));
		break;
	}

	if (errorList.isEmpty()) {
		delete resultDialog;
	}
	else {
		QString errorString;
		for (QString& error : errorList) {
			errorString += error + "\n";
		}
		resultDialog->setText(errorString);
		resultDialog->exec();
		errorList.clear();
	}

	// 复制操作完成后刷新表格
	if (type == OperationType::COPY) {
		emit requestRefreshMusic(PathType::LOCAL, tableManager->getCurrentPage(PathType::LOCAL));
		emit requestRefreshMusic(PathType::REMOTE, tableManager->getCurrentPage(PathType::REMOTE));
	}
}
