/**
 * @file MSTTagUtils.h
 * @brief TagLib 工具函数
 * @details 封装平台相关的 TagLib::FileRef 创建逻辑，
 *          消除多处重复的平台条件编译代码
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

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
