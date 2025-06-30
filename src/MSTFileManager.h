#pragma once
#include <QStorageInfo>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QFile>

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
};
