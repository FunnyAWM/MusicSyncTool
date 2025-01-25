#ifndef MUSICPROPERTIES_H
#define MUSICPROPERTIES_H

namespace PROPERTIES {
	enum class PathType { LOCAL, REMOTE };

	enum class PlayState { PLAYING, PAUSED, STOPPED };

	enum class FileErrorType { DUPLICATE, LNF, DISKFULL };

	// FNS : File Not Scannable
	enum class LoadErrorType { FNS, TAGERR };

	enum SortByEnum { TITLE = 0x00000001, ARTIST, ALBUM };

	enum OrderByEnum { ASC = 0x00000011, DESC };

	// LNF : Lyric Not Found
	// NPS : No Path Selected
	// NFS : No File Selected
	// PET : Program Error Type
	// NFT : No Favorite Tag
	// FIRST : First Page
	// LAST : Last Page
	enum class PET { NOAUDIO, NPS, NFT, NFS, FIRST, LAST };

	enum class OperationType { COPY, LOAD };

	enum class RuleField { TITLE, ARTIST, ALBUM };

	enum class RuleType { INCLUDES, EXCLUDES };

} // namespace PROPERTIES
#endif // MUSICPROPERTIES_H
