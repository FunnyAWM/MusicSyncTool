#pragma once
#include <QStorageInfo>

class MSTFileManager {
	QStorageInfo storageInfo;
public:
	MSTFileManager(QString);
	~MSTFileManager() = default;
	QString getSpaceInfo() const;
	bool copyable(const QString& filePath) const;
};
