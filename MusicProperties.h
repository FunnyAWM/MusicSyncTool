#ifndef MUSICPROPERTIES_H
#define MUSICPROPERTIES_H
namespace TOOLPROPERTIES
{
    // LNF : Lyric Not Found
    // NPS : No Path Selected
    // PET : Program Error Type
    enum class pathType { LOCAL, REMOTE };
    enum class playState { PLAYING, PAUSED, STOPPED };
    enum class fileErrorType { DUPLICATE, LNF };
    enum sortByEnum { TITLE = 1, ARTIST, ALBUM };
    enum class PET { NOAUDIO, NPS };
} // namespace MUSICPROPERTIES
#endif // MUSICPROPERTIES_H
