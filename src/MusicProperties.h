#ifndef MUSICPROPERTIES_H
#define MUSICPROPERTIES_H

namespace PROPERTIES {
	enum class pathType { LOCAL, REMOTE };

	enum class playState { PLAYING, PAUSED, STOPPED };

	enum class fileErrorType { DUPLICATE, LNF, DISKFULL };

	// FNS : File Not Scannable
	enum class loadErrorType { FNS, TAGERR };

	enum sortByEnum { TITLE = 0x00000001, ARTIST, ALBUM };

	enum orderByEnum { ASC = 0x00000011, DESC };

	// LNF : Lyric Not Found
	// NPS : No Path Selected
	// NFS : No File Selected
	// PET : Program Error Type
	// NFT : No Favorite Tag
	// FIRST : First Page
	// LAST : Last Page
	enum class PET { NOAUDIO, NPS, NFT, NFS, FIRST, LAST };

	enum class operationType { COPY, LOAD };

	enum class lyricRules { NAME, ARTIST, ALBUM };

	enum class ignoreLyricRules { INCLUDES, EXCLUDES };

} // namespace PROPERTIES
#endif // MUSICPROPERTIES_H
