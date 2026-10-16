#ifndef LOGGER_H
#define LOGGER_H
#include <QString>

/**
 * @brief 静态日志工具
 * @details 所有输出统一经 Qt 消息机制（qInfo/qDebug/qWarning/qCritical），
 *          便于统一收集；启用落盘后由消息处理器追加写入
 *          <baseDir>/log/lastRun.log（baseDir 见 MSTSettingsManager::baseDir）
 */
class Logger final {
public:
	/**
	 * @brief 启用或停用日志落盘
	 * @param enabled 是否写入日志文件
	 * @details 首次调用时安装 Qt 消息处理器，之后可反复切换开关
	 */
	static void setLogToFile(bool enabled);

	/// @brief 当前是否启用日志落盘
	[[nodiscard]] static bool isLogToFile();

	static void Info(const QString& message);

	static void Debug(const QString& message);

	static void Warn(const QString& message);

	static void Error(const QString& message);

	/// @brief 记录不可恢复错误并终止进程；仅在确实无法继续运行时使用
	static void Fatal(const QString& message);
};

#endif //LOGGER_H
