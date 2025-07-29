#pragma once
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>
#include <QStringList>
#include <functional>

/**
 * @brief 音乐同步工具文件管理器类
 * 负责管理存储空间信息和文件操作相关功能
 */
class MSTFileManager {
	QStorageInfo storageInfo; // 存储设备信息
	
public:
	// 支持的音频格式列表
	static const QStringList supportedFormat;
	/**
	 * @brief 构造函数
	 * @param path 文件路径
	 */
	MSTFileManager(QString path);
	
	/**
	 * @brief 析构函数
	 */
	~MSTFileManager() = default;
	
	/**
	 * @brief 获取存储空间信息
	 * @return 存储空间信息字符串（格式：可用空间/总空间）
	 */
	QString getSpaceInfo() const;
	
	/**
	 * @brief 检查文件是否可以复制（是否有足够的存储空间）
	 * @param filePath 文件路径
	 * @return 如果可以复制返回true，否则返回false
	 */
	bool copyable(const QString& filePath) const;
	
	/**
	 * @brief 检查文件格式是否受支持
	 * @param fileName 文件名
	 * @return 如果文件格式受支持返回true，否则返回false
	 * @details 检查文件扩展名是否在支持的音频格式列表中，
	 *          支持的格式包括常见的音频文件格式如mp3、flac、wav等
	 */
	static bool isFormatSupported(const QString& fileName);
	
	/**
	 * @brief 检查磁盘是否已满
	 * @param filePath 文件路径
	 * @param target 目标路径
	 * @return 如果磁盘空间不足返回true，否则返回false
	 * @details 比较要复制的文件大小与目标磁盘的可用空间，
	 *          判断是否有足够的空间进行文件复制操作
	 */
	static bool isFull(const QString& filePath, const QString& target);
	
	/**
	 * @brief 回滚复制操作
	 * @param fileName 要回滚的文件名
	 * @details 当文件复制失败时，清理已复制的文件，包括音乐文件本身
	 *          和对应的歌词文件（.lrc文件）。这确保了在出现错误时
	 *          不会留下不完整的文件副本
	 */
	static void rollBackCopy(const QString& fileName);
	
	/**
	 * @brief 复制音乐文件的核心逻辑（从MusicSyncTool分离出来）
	 * @param source 源目录路径
	 * @param fileList 要复制的文件列表（格式：文件名:规则命中状态）
	 * @param target 目标目录路径
	 * @param ignoreLyric 是否忽略歌词文件
	 * @param onProgress 进度回调函数 std::function<void(int)>
	 * @param onError 错误回调函数 std::function<void(const QString&, int)> (fileName, errorType)
	 * @param onStart 开始复制回调函数 std::function<void()>
	 * @param onFinish 完成复制回调函数 std::function<void()>
	 * @param onTotal 设置总数回调函数 std::function<void(int)>
	 * @details 这是从MusicSyncTool::copyMusic方法分离出来的核心逻辑，
	 *          通过回调函数处理UI更新和错误报告，保持原有的功能不变
	 */
	static void copyMusicFiles(const QString& source, const QStringList& fileList, 
	                          const QString& target, bool ignoreLyric,
	                          const std::function<void(int)>& onProgress = nullptr,
	                          const std::function<void(const QString&, int)>& onError = nullptr,
	                          const std::function<void()>& onStart = nullptr,
	                          const std::function<void()>& onFinish = nullptr,
	                          const std::function<void(int)>& onTotal = nullptr);
};
