#ifndef MSTFILEMANAGER_H
#define MSTFILEMANAGER_H
#include <functional>
#include <QStorageInfo>

class MSTFileManager {
	/// @brief 目标目录路径；存储信息每次使用时重新读取，避免过期快照
	QString storagePath;

public:
	/// 支持的音频格式列表
	static const QStringList supportedFormat;

	explicit MSTFileManager(const QString& path);

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
