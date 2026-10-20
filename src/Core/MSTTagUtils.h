#ifndef MSTTAGUTILS_H
#define MSTTAGUTILS_H

#include <QString>
#include <taglib/fileref.h>

namespace MSTTagUtils {
	/**
	 * @brief 根据平台创建 TagLib::FileRef
	 * @param filePath 音乐文件的完整路径
	 * @return TagLib::FileRef 对象
	 * @details Windows 下使用宽字符路径（toStdWString），
	 *          Linux 下使用标准字符路径（toStdString）
	 */
	inline TagLib::FileRef createFileRef(const QString& filePath) {
#if defined(_WIN64) or defined(_WIN32)
		return TagLib::FileRef(filePath.toStdWString().c_str());
#else
		return TagLib::FileRef(filePath.toStdString().c_str());
#endif
	}
}

#endif // MSTTAGUTILS_H
