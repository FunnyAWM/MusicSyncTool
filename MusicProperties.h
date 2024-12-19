#ifndef MUSICPROPERTIES_H
#define MUSICPROPERTIES_H
namespace TOOLPROPERTIES
{
    // LNF : Lyric Not Found
    // NPS : No Path Selected
    // NFS : No File Selected
    // PET : Program Error Type
    // NFT : No Favorite Tag
    enum class pathType { LOCAL, REMOTE };
    enum class playState { PLAYING, PAUSED, STOPPED };
    enum class fileErrorType { DUPLICATE, LNF };
    enum sortByEnum { TITLE = 1, ARTIST, ALBUM };
    enum orderByEnum { ASC = 11, DESC };
    enum class PET { NOAUDIO, NPS, NFT, NFS };
} // namespace MUSICPROPERTIES
#endif // MUSICPROPERTIES_H
