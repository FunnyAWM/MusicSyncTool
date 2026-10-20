#include "MSTFileManager.h"
#include <algorithm>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include "Logger.h"
#include "../Core/MSTTagUtils.h"

const QStringList MSTFileManager::supportedFormat = {
    "mp3", "flac", "wav", "aac", "ogg", "wma", "m4a", "ape", "aiff", "opus"
};

/// @brief 复制进度回调的最小间隔，避免大批量拷贝时高频信号冲刷事件队列
namespace {
	constexpr int progressIntervalMs = 200;
}

MSTFileManager::MSTFileManager(const QString& path) {
	storagePath = path;
}

/**
 * @return 格式化的存储空间信息，根据大小自动选择MB或GB单位
 */
QString MSTFileManager::getSpaceInfo() const {
	// 每次重新读取：磁盘空间会随复制过程变化，构造时的快照会显示过期数据
	const QStorageInfo storage(storagePath);
	const qsizetype totalSpace = storage.bytesTotal();
	const qsizetype availableSpace = storage.bytesAvailable();
	
	// 如果总空间小于1GB，使用MB单位显示
	if (static_cast<double>(totalSpace) / 1024.0 / 1024.0 / 1024.0 < 1) {
		return QString::number(static_cast<double>(availableSpace) / 1024.0 / 1024.0, 10, 2) + " MB / " + QString::number(
			static_cast<double>(totalSpace) / 1024.0 / 1024.0, 10, 2) + " MB";
	}
	// 否则使用GB单位显示
	return QString::number(static_cast<double>(availableSpace) / 1024.0 / 1024.0 / 1024.0, 10, 2) + " GB / " + QString::number(
		static_cast<double>(totalSpace) / 1024.0 / 1024.0 / 1024.0, 10, 2) + " GB";
}

/**
 * @brief 检查文件是否可以复制（检查存储空间是否足够）
 */
bool MSTFileManager::copyable(const QString& filePath) const {
	const QFileInfo fileInfo(filePath);
	const QStorageInfo storage(storagePath);
	return fileInfo.size() <= storage.bytesAvailable() && fileInfo.isFile();
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
    if (onTotal) onTotal(static_cast<int>(fileList.size()));
    
    const QDir targetDir(target);
    if (!targetDir.exists()) {
        if (!targetDir.mkpath(target)) {
            // 目标目录不可用时报告错误并结束，不再直接终止进程
            Logger::Error("Error creating directory: " + target);
            if (onError) onError(target, 2); // DISKFULL = 2
            if (onFinish) onFinish();
            return;
        }
    }
    const QDir sourceDir(source);

    // 进度回调节流：最多每 200ms 上报一次，避免大批量拷贝时冲刷事件队列
    QElapsedTimer progressTimer;
    progressTimer.start();
    const auto reportProgress = [&](const qsizetype index) {
        if (!onProgress || !progressTimer.hasExpired(progressIntervalMs)) {
            return;
        }
        onProgress(static_cast<int>(index));
        progressTimer.restart();
    };
    
    bool diskFull = false;
    // 用下标遍历：进度回调需要序号，indexOf 在大批量列表上是 O(n²) 且遇到重名会取错位置
    for (qsizetype index = 0; index < fileList.size(); ++index) {
        const QString& file = fileList.at(index);
        // 文件名以 ":<规则命中标记>" 结尾（见 MSTMainWindow::on_copyTo*_clicked），
        // 用最后一个冒号切分；缺少该后缀时按整体文件名处理，避免 at() 越界抛异常
        const qsizetype separator = file.lastIndexOf(':');
        const QString fileName = separator >= 0 ? file.left(separator) : file;
        const bool ruleHit = separator >= 0 && file.mid(separator + 1).toInt() != 0;

        const QString sourceFile = sourceDir.filePath(fileName);
        const QString targetFile = targetDir.filePath(fileName);
        
        if (diskFull) {
            if (onError) onError(fileName, 2); // DISKFULL = 2
            continue;
        }
        
        // 歌词文件与音频文件同名、扩展名为 .lrc；
        // 用 QFileInfo 派生，避免对整条路径做子串替换时误伤目录名
        const QFileInfo sourceInfo(sourceFile);
        const QFileInfo targetInfo(targetFile);
        const QString lyric = sourceInfo.dir().filePath(sourceInfo.completeBaseName() + ".lrc");
        const QString lyricTarget = targetInfo.dir().filePath(targetInfo.completeBaseName() + ".lrc");

        // 递归扫描得到的相对路径可能带子目录，复制前先建好目标子目录
        if (targetInfo.dir().path() != targetDir.path() && !targetInfo.dir().exists()
            && !targetInfo.dir().mkpath(".")) {
            Logger::Error("Error creating directory: " + targetInfo.absolutePath());
            if (onError) onError(fileName, 2); // DISKFULL = 2
            continue;
        }
        
        if (QFile::exists(targetFile)) {
            Logger::Warn("File existed, skipping " + targetFile);
            if (onError) onError(fileName, 0); // DUPLICATE = 0
            continue;
        }
        
        if (!ignoreLyric && !ruleHit && !QFile::exists(lyric)) {
            // 歌词文件缺失：读取音频标签，内嵌歌词存在时仍可复制
            const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(sourceFile);
            if (!fileRef.isNull() && fileRef.tag()) {
                const TagLib::Tag* tag = fileRef.tag();
                if (!tag->properties().contains(key)) {
                    Logger::Warn("Lyric file not found, skipping " + lyric);
                    if (onError) onError(fileName, 1); // LNF = 1
                    continue;
                }
            }
        }
        
        diskFull = isFull(sourceFile, target);
        if (diskFull) {
            rollBackCopy(targetFile);
            if (onError) onError(fileName, 2); // DISKFULL = 2
            continue;
        }
        
        // 先复制音频文件
        if (!QFile::copy(sourceFile, targetFile)) {
            rollBackCopy(targetFile);
            if (onError) onError(fileName, 2); // DISKFULL = 2
            continue;
        }

        // 存在同名歌词文件时一并复制；失败则回滚已复制的音频
        if (!ignoreLyric && QFile::exists(lyric) && !QFile::copy(lyric, lyricTarget)) {
            rollBackCopy(targetFile);
            if (onError) onError(fileName, 2); // DISKFULL = 2
            continue;
        }

        reportProgress(index);
    }

    // 收尾补一次进度，避免节流导致进度条停在中间
    if (onProgress && !fileList.isEmpty()) {
        onProgress(static_cast<int>(fileList.size()));
    }
    
    if (onFinish) onFinish();
}
