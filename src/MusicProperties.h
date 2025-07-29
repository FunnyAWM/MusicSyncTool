#ifndef MUSICPROPERTIES_H
#define MUSICPROPERTIES_H

/**
 * @brief 音乐属性命名空间
 * 包含音乐同步工具中使用的各种枚举类型和工具函数
 */
namespace PROPERTIES {
	/**
	 * @brief 路径类型枚举
	 */
	enum class PathType { 
		LOCAL,   // 本地路径
		REMOTE   // 远程路径
	};

	/**
	 * @brief 播放状态枚举
	 */
	enum class PlayState { 
		PLAYING,  // 正在播放
		PAUSED,   // 已暂停
		STOPPED   // 已停止
	};

	/**
	 * @brief 文件错误类型枚举
	 */
	enum class FileErrorType { 
		DUPLICATE,  // 重复文件
		LNF,        // 文件未找到 (Lyric Not Found)
		DISKFULL    // 磁盘空间不足
	};

	/**
	 * @brief 加载错误类型枚举
	 * FNS: File Not Scannable (文件无法扫描)
	 */
	enum class LoadErrorType { 
		FNS,     // 文件无法扫描
		TAGERR   // 标签错误
	};

	/**
	 * @brief 排序字段枚举
	 */
	enum class SortByEnum { 
		TITLE,   // 按标题排序
		ARTIST,  // 按艺术家排序
		ALBUM    // 按专辑排序
	};

	/**
	 * @brief 将数字转换为排序字段枚举
	 * @param num 数字值
	 * @return 对应的排序字段枚举
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
     * @brief 将排序字段枚举转换为数字
     * @param sortBy 排序字段枚举
     * @return 对应的数字值
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
	 * @brief 排序顺序枚举
	 */
	enum class OrderByEnum { 
		ASC,   // 升序
		DESC   // 降序
	};

	/**
	 * @brief 将数字转换为排序顺序枚举
	 * @param num 数字值
	 * @return 对应的排序顺序枚举
	 */
    inline OrderByEnum toOrderBy(const short num) {
        switch (num) {
            case 0x11: return OrderByEnum::ASC;
            case 0x12: return OrderByEnum::DESC;
        default: return OrderByEnum::ASC;
        }
    }

    /**
     * @brief 将排序顺序枚举转换为数字
     * @param orderBy 排序顺序枚举
     * @return 对应的数字值
     */
    inline short toShort(const OrderByEnum orderBy) {
        switch (orderBy) {
            case OrderByEnum::ASC: return 0x11;
            case OrderByEnum::DESC: return 0x12;
            default: return 0x11;
        }
    }
    
	/**
	 * @brief 程序错误类型枚举 (Program Error Type)
	 * LNF: Lyric Not Found (歌词未找到)
	 * NPS: No Path Selected (未选择路径)
	 * NFS: No File Selected (未选择文件)
	 * NFT: No Favorite Tag (无收藏标签)
	 * FIRST: First Page (第一页)
	 * LAST: Last Page (最后一页)
	 * RUNNING: Program Already Running (程序已在运行)
	 * DOF: Directory open failed (目录打开失败)
	 */
	enum class PET { 
		NOAUDIO,  // 无音频设备
		NPS,      // 未选择路径
		NFT,      // 无收藏标签
		NFS,      // 未选择文件
		FIRST,    // 第一页
		LAST,     // 最后一页
		RUNNING,  // 程序已运行
		NOLANG,   // 无语言设置
		DBERROR,  // 数据库错误
		DOF       // 目录打开失败
	};

	/**
	 * @brief 操作类型枚举
	 */
	enum class OperationType { 
		COPY,  // 复制操作
		LOAD   // 加载操作
	};

	/**
	 * @brief 规则字段枚举
	 */
	enum class RuleField { 
		TITLE,   // 标题字段
		ARTIST,  // 艺术家字段
		ALBUM    // 专辑字段
	};

	/**
	 * @brief 规则类型枚举
	 */
	enum class RuleType { 
		INCLUDES,  // 包含规则
		EXCLUDES   // 排除规则
	};

	/**
	 * @brief 查询行枚举
	 */
	enum class QueryRows { 
		TITLE,     // 标题行
		ARTIST,    // 艺术家行
		ALBUM,     // 专辑行
		GENRE,     // 流派行
		YEAR,      // 年份行
		TRACK,     // 音轨行
		FILENAME,  // 文件名行
		ALL        // 所有行
	};

	/**
	 * @brief 日志文件输出枚举
	 */
	enum class LogToFile { 
		ENABLE,   // 启用日志文件
		DISABLE   // 禁用日志文件
	};
} // namespace PROPERTIES
#endif // MUSICPROPERTIES_H
