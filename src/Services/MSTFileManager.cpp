#include "MSTFileManager.h"
#include <algorithm>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include "Logger.h"

const QStringList MSTFileManager::supportedFormat = {
    "mp3", "flac", "wav", "aac", "ogg", "wma", "m4a", "ape", "aiff", "opus"
};

MSTFileManager::MSTFileManager(const QString& path) {
	storageInfo = QStorageInfo(path);
}

/**
 * @return 格式化的存储空间信息，根据大小自动选择MB或GB单位
 */
QString MSTFileManager::getSpaceInfo() const {
	const qsizetype totalSpace = storageInfo.bytesTotal();
	const qsizetype availableSpace = storageInfo.bytesAvailable();
	
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
 */
bool MSTFileManager::copyable(const QString& filePath) const {
	const QFileInfo fileInfo(filePath);
	return fileInfo.size() <= storageInfo.bytesAvailable() && fileInfo.isFile();
}

bool MSTFileManager::isFormatSupported(const QString& fileName) {
    return std::any_of(supportedFormat.begin(), supportedFormat.end(), [&fileName](const QString& format) {
        const QString extension = fileName.section('.', -1);
        return extension.compare(format, Qt::CaseInsensitive) == 0;
    });
}

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
    if (!dir.exists()) {
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

                // 先复制音频文件
                if (!QFile::copy(sourceFile, targetFile)) {
                    rollBackCopy(targetFile);
                    if (onError) onError(fileParts.at(0), 2); // DISKFULL = 2
                    continue;
                }

                // 再复制歌词文件；失败则回滚已复制的音频
                if (!QFile::copy(lyric, lyricTarget)) {
                    rollBackCopy(targetFile);
                    if (onError) onError(fileParts.at(0), 2); // DISKFULL = 2
                    continue;
                }

                if (onProgress) onProgress(fileList.indexOf(file));
                continue;
            }
        }
        
        diskFull = isFull(sourceFile, target);
        if (diskFull) {
            rollBackCopy(targetFile);
            if (onError) onError(fileParts.at(0), 2); // DISKFULL = 2
            continue;
        }
        
        if (!QFile::copy(sourceFile, targetFile)) {
            rollBackCopy(targetFile);
            if (onError) onError(fileParts.at(0), 2); // DISKFULL = 2
            continue;
        }
        if (onProgress) onProgress(fileList.indexOf(file));
    }
    
    if (onFinish) onFinish();
}
