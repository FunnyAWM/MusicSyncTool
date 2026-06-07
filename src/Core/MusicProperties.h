/**
 * @file MusicProperties.h
 * @brief 音乐同步工具核心枚举类型与工具函数定义
 * @details 定义了应用程序中使用的所有枚举类型，包括路径类型、播放状态、
 *          排序方式、规则类型等，以及枚举与数值之间的转换工具函数
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MUSICPROPERTIES_H
#define MUSICPROPERTIES_H

/**
 * @brief 音乐属性命名空间
 * 包含音乐同步工具中使用的各种枚举类型和工具函数
 */
namespace PROPERTIES {
	/**
	 * @brief 路径类型枚举
	 * 用于区分本地路径和远程路径
	 */
	enum class PathType { 
		LOCAL,   ///< 本地路径
		REMOTE   ///< 远程路径
	};

	/**
	 * @brief 播放状态枚举
	 * 表示媒体播放器的当前状态
	 */
	enum class PlayState { 
		PLAYING,  ///< 正在播放
		PAUSED,   ///< 已暂停
		STOPPED   ///< 已停止
	};

	/**
	 * @brief 文件错误类型枚举
	 * 文件复制操作中可能遇到的错误类型
	 */
	enum class FileErrorType { 
		DUPLICATE,  ///< 重复文件
		LNF,        ///< 歌词文件未找到 (Lyric Not Found)
		DISKFULL    ///< 磁盘空间不足
	};

	/**
	 * @brief 加载错误类型枚举
	 * 音乐文件加载过程中可能遇到的错误类型
	 */
	enum class LoadErrorType { 
		FNS,     ///< 文件无法扫描 (File Not Scannable)
		TAGERR   ///< 标签读取错误
	};

	/**
	 * @brief 排序字段枚举
	 * 指定音乐列表排序所依据的字段
	 */
	enum class SortByEnum { 
		TITLE,   ///< 按标题排序
		ARTIST,  ///< 按艺术家排序
		ALBUM    ///< 按专辑排序
	};

	/**
	 * @brief 排序顺序枚举
	 * 指定排序的升降序方向
	 */
	enum class OrderByEnum { 
		ASC,   ///< 升序
		DESC   ///< 降序
	};

	/**
	 * @brief 程序错误类型枚举 (Program Error Type)
	 * 应用程序运行时可能遇到的各类错误
	 */
	enum class PET { 
		NOAUDIO,  ///< 无音频设备或未选定音频
		NPS,      ///< 未选择路径 (No Path Selected)
		NDP,      ///< 未选择目标路径 (No Destination Path)
		NFT,      ///< 无收藏标签 (No Favorite Tag)
		NFS,      ///< 未选择文件 (No File Selected)
		FIRST,    ///< 已经是第一页
		LAST,     ///< 已经是最后一页
		RUNNING,  ///< 程序已在运行
		NOLANG,   ///< 找不到语言配置文件
		DBERROR,  ///< 数据库操作错误
		DOF       ///< 目录打开失败 (Directory Open Failed)
	};

	/**
	 * @brief 操作类型枚举
	 * 用于区分不同的异步操作类型
	 */
	enum class OperationType { 
		COPY,  ///< 文件复制操作
		LOAD   ///< 数据加载操作
	};

	/**
	 * @brief 规则字段枚举
	 * 歌词忽略规则所作用的字段
	 */
	enum class RuleField { 
		TITLE,   ///< 标题字段
		ARTIST,  ///< 艺术家字段
		ALBUM    ///< 专辑字段
	};

	/**
	 * @brief 规则类型枚举
	 * 歌词忽略规则的匹配方式
	 */
	enum class RuleType { 
		INCLUDES,  ///< 包含匹配
		EXCLUDES   ///< 排除匹配
	};

	/**
	 * @brief 查询行枚举
	 * 指定数据库查询时需要返回的字段
	 */
	enum class QueryRows { 
		TITLE,     ///< 标题行
		ARTIST,    ///< 艺术家行
		ALBUM,     ///< 专辑行
		GENRE,     ///< 流派行
		YEAR,      ///< 年份行
		TRACK,     ///< 音轨行
		FILENAME,  ///< 文件名行
		ALL        ///< 所有行
	};

	/**
	 * @brief 日志文件输出枚举
	 * 控制日志是否同时输出到文件
	 */
	enum class LogToFile { 
		ENABLE,   ///< 启用日志文件输出
		DISABLE   ///< 禁用日志文件输出
	};

	// ==================== 枚举转换工具函数 ====================

	/**
	 * @brief 将数值转换为排序字段枚举
	 * @param num 数值（0x01=标题, 0x02=艺术家, 0x03=专辑）
	 * @return 对应的排序字段枚举值
	 */
    inline SortByEnum toSortBy(const short num) {
        switch (num) {
            case 0x01: return SortByEnum::TITLE;
            case 0x02: return SortByEnum::ARTIST;
            case 0x03: return SortByEnum::ALBUM;
            default: return SortByEnum::TITLE;
            }
    }

    /**
     * @brief 将排序字段枚举转换为数值
     * @param sortBy 排序字段枚举值
     * @return 对应的数值（0x01=标题, 0x02=艺术家, 0x03=专辑）
     */
    inline short toShort(const SortByEnum sortBy) {
        switch (sortBy) {
            case SortByEnum::TITLE: return 0x01;
            case SortByEnum::ARTIST: return 0x02;
            case SortByEnum::ALBUM: return 0x03;
            default: return 0x01;
        }
    }

	/**
	 * @brief 将数值转换为排序顺序枚举
	 * @param num 数值（0x11=升序, 0x12=降序）
	 * @return 对应的排序顺序枚举值
	 */
    inline OrderByEnum toOrderBy(const short num) {
        switch (num) {
            case 0x11: return OrderByEnum::ASC;
            case 0x12: return OrderByEnum::DESC;
        default: return OrderByEnum::ASC;
        }
    }

    /**
     * @brief 将排序顺序枚举转换为数值
     * @param orderBy 排序顺序枚举值
     * @return 对应的数值（0x11=升序, 0x12=降序）
     */
    inline short toShort(const OrderByEnum orderBy) {
        switch (orderBy) {
            case OrderByEnum::ASC: return 0x11;
            case OrderByEnum::DESC: return 0x12;
            default: return 0x11;
        }
    }
} // namespace PROPERTIES
#endif // MUSICPROPERTIES_H
