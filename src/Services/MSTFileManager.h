/**
 * @file MSTFileManager.h
 * @brief 音乐同步工具文件管理器类定义
 * @details 定义了管理存储空间信息和文件操作的类，
 *          包括磁盘空间检测、音频格式支持检查、文件复制和回滚等功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTFILEMANAGER_H
#define MSTFILEMANAGER_H
#include <functional>
#include <QStorageInfo>

/**
 * @brief 音乐同步工具文件管理器类
 * 负责管理存储空间信息和文件操作相关功能，
 * 包括磁盘空间检测、音频格式检查、文件复制和错误回滚
 */
class MSTFileManager {
	QStorageInfo storageInfo; ///< 存储设备信息

public:
	/// 支持的音频格式列表
	static const QStringList supportedFormat;

	explicit MSTFileManager(const QString& path);

	/**
	 * @brief 析构函数
	 */
	~MSTFileManager() = default;

	QString getSpaceInfo() const;

	bool copyable(const QString& filePath) const;

	static bool isFormatSupported(const QString& fileName);

	static bool isFull(const QString& filePath, const QString& target);

	static void rollBackCopy(const QString& fileName);

	static void copyMusicFiles(const QString& source, const QStringList& fileList,
	                           const QString& target, bool ignoreLyric,
	                           const std::function<void(int)>& onProgress = nullptr,
	                           const std::function<void(const QString&, int)>& onError = nullptr,
	                           const std::function<void()>& onStart = nullptr,
	                           const std::function<void()>& onFinish = nullptr,
	                           const std::function<void(int)>& onTotal = nullptr);
};

#endif // MSTFILEMANAGER_H
