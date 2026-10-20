#ifndef MSTFILEMANAGER_H
#define MSTFILEMANAGER_H
#include <atomic>
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

	/**
	 * @brief 批量复制音乐文件及其歌词
	 * @param shouldCancel 取消标志，由调用方线程置位；仅在每首歌曲开始前读取，
	 *                     因此当前歌曲与其歌词必然完整复制后才退出；传 nullptr 表示不可取消
	 * @details 取消不是回滚：已完成的文件保留，剩余文件不再开始复制
	 */
	static void copyMusicFiles(const QString& source, const QStringList& fileList,
	                           const QString& target, bool ignoreLyric,
	                           const std::function<void(int)>& onProgress = nullptr,
	                           const std::function<void(const QString&, int)>& onError = nullptr,
	                           const std::function<void()>& onStart = nullptr,
	                           const std::function<void()>& onFinish = nullptr,
	                           const std::function<void(int)>& onTotal = nullptr,
	                           const std::atomic<bool>* shouldCancel = nullptr);
};

#endif // MSTFILEMANAGER_H
