#ifndef MUSICPROPERTIES_H
#define MUSICPROPERTIES_H

namespace PROPERTIES {
	enum class PathType { LOCAL, REMOTE };

	enum class PlayState { PLAYING, PAUSED, STOPPED };

	enum class FileErrorType { DUPLICATE, LNF, DISKFULL };

	// FNS : File Not Scannable
	enum class LoadErrorType { FNS, TAGERR };

	enum class SortByEnum { TITLE, ARTIST, ALBUM };

    inline SortByEnum toSortBy(const short num) {
        switch (num) {
            case 0x01: return SortByEnum::TITLE;
            case 0x02: return SortByEnum::ARTIST;
            case 0x03: return SortByEnum::ALBUM;
            default: return SortByEnum::TITLE;
            }
    }

    inline short toShort(const SortByEnum sortBy) {
        switch (sortBy) {
            case SortByEnum::TITLE: return 0x01;
            case SortByEnum::ARTIST: return 0x02;
            case SortByEnum::ALBUM: return 0x03;
            default: return 0x01;
        }
    }

	enum class OrderByEnum { ASC, DESC };


    inline OrderByEnum toOrderBy(const short num) {
        switch (num) {
            case 0x11: return OrderByEnum::ASC;
            case 0x12: return OrderByEnum::DESC;
        default: return OrderByEnum::ASC;
        }
    }

    inline short toShort(const OrderByEnum orderBy) {
        switch (orderBy) {
            case OrderByEnum::ASC: return 0x11;
            case OrderByEnum::DESC: return 0x12;
            default: return 0x11;
        }
    }
	// PET : Program Error Type
	// LNF : Lyric Not Found
	// NPS : No Path Selected
	// NFS : No File Selected
	// NFT : No Favorite Tag
	// FIRST : First Page
	// LAST : Last Page
	// RUNNING : Program Already Running
	// DOF : Directory open failed
	enum class PET { NOAUDIO, NPS, NFT, NFS, FIRST, LAST, RUNNING, NOLANG, DBERROR, DOF };

	enum class OperationType { COPY, LOAD };

	enum class RuleField { TITLE, ARTIST, ALBUM };

	enum class RuleType { INCLUDES, EXCLUDES };

	enum class QueryRows { TITLE, ARTIST, ALBUM, GENRE, YEAR, TRACK, FILENAME, ALL };

	enum class LogToFile { ENABLE, DISABLE };
} // namespace PROPERTIES
#endif // MUSICPROPERTIES_H
