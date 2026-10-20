#include "TestMusicProperties.h"

void TestMusicProperties::toSortBy_valid() {
    QCOMPARE(toSortBy(static_cast<short>(0x01)), SortByEnum::TITLE);
    QCOMPARE(toSortBy(static_cast<short>(0x02)), SortByEnum::ARTIST);
    QCOMPARE(toSortBy(static_cast<short>(0x03)), SortByEnum::ALBUM);
}

void TestMusicProperties::toSortBy_default() {
    QCOMPARE(toSortBy(static_cast<short>(0x00)), SortByEnum::TITLE);
    QCOMPARE(toSortBy(static_cast<short>(0xFF)), SortByEnum::TITLE);
    QCOMPARE(toSortBy(static_cast<short>(-1)), SortByEnum::TITLE);
}

void TestMusicProperties::toSortBy_boundary() {
    QCOMPARE(toSortBy(static_cast<short>(0x04)), SortByEnum::TITLE);
    QCOMPARE(toSortBy(static_cast<short>(0x10)), SortByEnum::TITLE);
}

void TestMusicProperties::toShort_sortBy() {
    QCOMPARE(toShort(SortByEnum::TITLE),  static_cast<short>(0x01));
    QCOMPARE(toShort(SortByEnum::ARTIST), static_cast<short>(0x02));
    QCOMPARE(toShort(SortByEnum::ALBUM),  static_cast<short>(0x03));
}

void TestMusicProperties::toShort_sortBy_default() {
    QCOMPARE(toShort(static_cast<SortByEnum>(999)), static_cast<short>(0x01));
}

void TestMusicProperties::toOrderBy_valid() {
    QCOMPARE(toOrderBy(static_cast<short>(0x11)), OrderByEnum::ASC);
    QCOMPARE(toOrderBy(static_cast<short>(0x12)), OrderByEnum::DESC);
}

void TestMusicProperties::toOrderBy_default() {
    QCOMPARE(toOrderBy(static_cast<short>(0x00)), OrderByEnum::ASC);
    QCOMPARE(toOrderBy(static_cast<short>(0xFF)), OrderByEnum::ASC);
}

void TestMusicProperties::toShort_orderBy() {
    QCOMPARE(toShort(OrderByEnum::ASC),  static_cast<short>(0x11));
    QCOMPARE(toShort(OrderByEnum::DESC), static_cast<short>(0x12));
}

void TestMusicProperties::sortBy_roundtrip() {
    for (auto e : {SortByEnum::TITLE, SortByEnum::ARTIST, SortByEnum::ALBUM})
        QCOMPARE(toSortBy(toShort(e)), e);
    for (short n : {static_cast<short>(0x01), static_cast<short>(0x02), static_cast<short>(0x03)})
        QCOMPARE(toShort(toSortBy(n)), n);
}

void TestMusicProperties::orderBy_roundtrip() {
    for (auto e : {OrderByEnum::ASC, OrderByEnum::DESC})
        QCOMPARE(toOrderBy(toShort(e)), e);
    for (short n : {static_cast<short>(0x11), static_cast<short>(0x12)})
        QCOMPARE(toShort(toOrderBy(n)), n);
}

void TestMusicProperties::themeMode_roundtrip() {
    for (auto e : {ThemeMode::LIGHT, ThemeMode::DARK})
        QCOMPARE(toThemeMode(toShort(e)), e);
    for (short n : {static_cast<short>(0x21), static_cast<short>(0x22)})
        QCOMPARE(toShort(toThemeMode(n)), n);
}

void TestMusicProperties::themeMode_default() {
    QCOMPARE(toThemeMode(static_cast<short>(0x00)), ThemeMode::LIGHT);
    QCOMPARE(toThemeMode(static_cast<short>(0xFF)), ThemeMode::LIGHT);
}

void TestMusicProperties::accentColor_roundtrip() {
    for (auto e : {AccentColor::BLUE, AccentColor::GREEN, AccentColor::PURPLE, AccentColor::DARK})
        QCOMPARE(toAccentColor(toShort(e)), e);
    for (short n : {static_cast<short>(0x31), static_cast<short>(0x32),
                    static_cast<short>(0x33), static_cast<short>(0x34)})
        QCOMPARE(toShort(toAccentColor(n)), n);
}

void TestMusicProperties::accentColor_default() {
    // 越界编码回退蓝色（style.qss），与 toAccentColor 的 default 分支保持一致
    QCOMPARE(toAccentColor(static_cast<short>(0x00)), AccentColor::BLUE);
    QCOMPARE(toAccentColor(static_cast<short>(0xFF)), AccentColor::BLUE);
}