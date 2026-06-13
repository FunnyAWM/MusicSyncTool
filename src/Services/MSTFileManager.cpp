/**
 * @file MSTFileManager.cpp
 * @brief 音乐同步工具文件管理器类的实现
 * @details 实现存储空间查询、音频格式检查、文件复制和错误回滚等文件管理功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTFileManager.h"
#include <algorithm>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include "Logger.h"

/// 支持的音频格式列表常量定义
const QStringList MSTFileManager::supportedFormat = {
    "mp3", "flac", "wav", "aac", "ogg", "wma", "m4a", "ape", "aiff", "opus"
};

/**
 * @brief 构造函数，初始化存储信息
 * @param path 存储路径
 */
MSTFileManager::MSTFileManager(const QString& path) {
	storageInfo = QStorageInfo(path);
}

/**
 * @brief 获取存储空间信息字符串
 * @return 格式化的存储空间信息，根据大小自动选择MB或GB单位
 */
QString MSTFileManager::getSpaceInfo() const {
	const qsizetype totalSpace = storageInfo.bytesTotal();       // 总空间
	const qsizetype availableSpace = storageInfo.bytesAvailable(); // 可用空间
	
	// 如果总空间小于1GB，使用MB单位显示
	if (static_cast<double>(totalSpace) / 1024.0 / 1024.0 / 1024.0 < 1) {
		return QString::number(static_cast<double>(availableSpace) / 1024.0 / 1024.0, 10, 2) + " MB / " + QString::number(
			static_cast<double>(totalSpace) / 1024.0 / 1024.0, 10, 2) + " MB";
	}
	// 否则使用GB单位显示
	return QString::number(static_cast<double>(availableSpace) / 1024.0 / 1024.0 / 1024.0, 10, 2) + " GB / " + QString::number(
		totalSpace / 1024.0 / 1024.0 / 1024.0, 10, 2) + " GB";
}

/**
 * @brief 检查文件是否可以复制（检查存储空间是否足够）
 * @param filePath 要检查的文件路径
 * @return 如果文件大小未超过可用空间且是有效文件返回true，否则返回false
 */
bool MSTFileManager::copyable(const QString& filePath) const {
	const QFileInfo fileInfo(filePath);
	return fileInfo.size() <= storageInfo.bytesAvailable() && fileInfo.isFile();
}

/**
 * @brief 检查文件格式是否受支持
 * @param fileName 文件名
 * @return 如果文件格式受支持返回true，否则返回false
 */
bool MSTFileManager::isFormatSupported(const QString& fileName) {
    return std::any_of(supportedFormat.begin(), supportedFormat.end(), [&fileName](const QString& format) {
        const QString extension = fileName.section('.', -1);
        return extension.compare(format, Qt::CaseInsensitive) == 0;
    });
}

/**
 * @brief 检查磁盘是否已满
 * @param filePath 文件路径
 * @param target 目标路径
 * @return 如果磁盘空间不足返回true，否则返回false
 */
bool MSTFileManager::isFull(const QString& filePath, const QString& target) {
    const QFileInfo musicInfo(filePath);
    const QDir targetInfo(target);
    const QStorageInfo storage(targetInfo);
    if ( musicInfo.size() > storage.bytesAvailable()) {
        return true;
    }
    return false;
}

/**
 * @brief 回滚复制操作
 * @param fileName 要回滚的文件名
 * @details 当文件复制失败时，清理已复制的文件，包括音乐文件本身
 *          和对应的歌词文件（.lrc文件）
 */
void MSTFileManager::rollBackCopy(const QString& fileName) {
    const QString lyric = fileName.section('.', 0, -2) + ".lrc";
    if (QFile::exists(lyric)) {
        QFile::remove(lyric);
    }
    if (QFile::exists(fileName)) {
        QFile::remove(fileName);
    }
}

/**
 * @brief 复制音乐文件的核心逻辑
 * @details 遍历文件列表，逐个复制音乐文件及其对应的歌词文件。
 *          处理文件已存在、歌词未找到、磁盘满等错误情况
 */
void MSTFileManager::copyMusicFiles(const QString& source, const QStringList& fileList, 
                                   const QString& target, bool ignoreLyric,
                                   const std::function<void(int)>& onProgress,
                                   const std::function<void(const QString&, int)>& onError,
                                   const std::function<void()>& onStart,
                                   const std::function<void()>& onFinish,
                                   const std::function<void(int)>& onTotal) {
    const TagLib::String key = "LYRICS";
    
    if (onStart) onStart();
    if (onTotal) onTotal(fileList.size());
    
    const QDir dir(target);
    if (dir.isEmpty()) {
        if (!dir.mkpath(target)) {
            Logger::Fatal("Error creating directory: " + target);
            return;
        }
    }
    
    bool diskFull = false;
    for (const QString& file : fileList) {
        QStringList fileParts = file.split(":");
        QString sourceFile = source + "/" + fileParts.at(0);
        QString targetFile = target + "/" + fileParts.at(0);
        
        if (diskFull) {
            if (onError) onError(fileParts.at(0), 2); // DISKFULL = 2
            continue;
        }
        
        QString lyric;
        QString lyricTarget;
        for (const QString& format : supportedFormat) {
            if (sourceFile.contains(format)) {
                QString lyricSourcePath = sourceFile;
                QString lyricTargetPath = targetFile;
                lyric = lyricSourcePath.replace(format, "lrc");
                lyricTarget = lyricTargetPath.replace(format, "lrc");
                break;
            }
        }
        
        if (QFile::exists(targetFile)) {
            Logger::Warn("File existed, skipping " + targetFile);
            if (onError) onError(fileParts.at(0), 0); // DUPLICATE = 0
            continue;
        }
        
        if (!ignoreLyric && !static_cast<bool>(fileParts.at(1).toInt())) {
            if (!QFile::exists(lyric)) {
                TagLib::FileRef fileRef;
#if defined(_WIN64) or defined(_WIN32)
                fileRef = TagLib::FileRef(sourceFile.toStdWString().c_str());
#else
                fileRef = TagLib::FileRef(sourceFile.toStdString().c_str());
#endif
                if (!fileRef.isNull() && fileRef.tag()) {
                    const TagLib::Tag* tag = fileRef.tag();
                    if (!tag->properties().contains(key)) {
                        Logger::Warn("Lyric file not found, skipping " + lyric);
                        if (onError) onError(fileParts.at(0), 1); // LNF = 1
                        continue;
                    }
                }
            } else {
                diskFull = isFull(sourceFile, target);
                if (diskFull) {
                    rollBackCopy(targetFile);
                    if (onError) onError(fileParts.at(0), 2); // DISKFULL = 2
                    continue;
                }
                QFile::copy(lyric, lyricTarget);
            }
        }
        
        diskFull = isFull(sourceFile, target);
        if (diskFull) {
            rollBackCopy(targetFile);
            if (onError) onError(fileParts.at(0), 2); // DISKFULL = 2
            continue;
        }
        
        QFile::copy(sourceFile, targetFile);
        if (onProgress) onProgress(fileList.indexOf(file));
    }
    
    if (onFinish) onFinish();
}
