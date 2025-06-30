#include "MSTFileManager.h"

MSTFileManager::MSTFileManager(QString path) {
	storageInfo = QStorageInfo(path);
}

QString MSTFileManager::getSpaceInfo() const {
	const qsizetype totalSpace = storageInfo.bytesTotal();
	const qsizetype availableSpace = storageInfo.bytesAvailable();
	if (static_cast<double>(totalSpace) / 1024.0 / 1024.0 / 1024.0 < 1) {
		return QString::number(static_cast<double>(availableSpace) / 1024.0 / 1024.0, 10, 2) + " MB / " + QString::number(
			static_cast<double>(totalSpace) / 1024.0 / 1024.0, 10, 2) + " MB";
	}
	return QString::number(static_cast<double>(availableSpace) / 1024.0 / 1024.0 / 1024.0, 10, 2) + " GB / " + QString::number(
		totalSpace / 1024.0 / 1024.0 / 1024.0, 10, 2) + " GB";
}

bool MSTFileManager::copyable(const QString& filePath) const {
	const QFileInfo fileInfo(filePath);
	return fileInfo.size() > storageInfo.bytesAvailable() && fileInfo.isFile();
}
