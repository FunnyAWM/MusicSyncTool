/**
 * @file TestMusicProperties.cpp
 * @brief White-box branch coverage tests for PROPERTIES namespace enums and
 *        conversion functions defined in src/Core/MusicProperties.h
 *
 * Branch coverage targets:
 *   toSortBy  : case 0x01 / 0x02 / 0x03 / default
 *   toShort(SortByEnum)  : case TITLE / ARTIST / ALBUM / default
 *   toOrderBy : case 0x11 / 0x12 / default
 *   toShort(OrderByEnum) : case ASC / DESC / default
 *   Round-trip consistency for both enum families
 */

#include "TestMusicProperties.h"

// ═══════════════════════════════════════════════════════════════════════════
// toSortBy – all explicit case branches + default fallback
// ═══════════════════════════════════════════════════════════════════════════

void TestMusicProperties::testToSortBy_validValues()
{
    // Branch: case 0x01 → TITLE
    QCOMPARE(toSortBy(static_cast<short>(0x01)), SortByEnum::TITLE);

    // Branch: case 0x02 → ARTIST
    QCOMPARE(toSortBy(static_cast<short>(0x02)), SortByEnum::ARTIST);

    // Branch: case 0x03 → ALBUM
    QCOMPARE(toSortBy(static_cast<short>(0x03)), SortByEnum::ALBUM);
}

void TestMusicProperties::testToSortBy_invalidValue_returnsDefault()
{
    // Branch: default → TITLE (fallback for any value not in {0x01,0x02,0x03})
    QCOMPARE(toSortBy(static_cast<short>(0x00)), SortByEnum::TITLE);
    QCOMPARE(toSortBy(static_cast<short>(0x04)), SortByEnum::TITLE);
    QCOMPARE(toSortBy(static_cast<short>(-1)),   SortByEnum::TITLE);
    QCOMPARE(toSortBy(static_cast<short>(0xFF)), SortByEnum::TITLE);
}

// ═══════════════════════════════════════════════════════════════════════════
// toShort(SortByEnum) – all case branches
// ═══════════════════════════════════════════════════════════════════════════

void TestMusicProperties::testToShort_sortBy()
{
    // Branch: TITLE  → 0x01
    QCOMPARE(toShort(SortByEnum::TITLE),  static_cast<short>(0x01));

    // Branch: ARTIST → 0x02
    QCOMPARE(toShort(SortByEnum::ARTIST), static_cast<short>(0x02));

    // Branch: ALBUM  → 0x03
    QCOMPARE(toShort(SortByEnum::ALBUM),  static_cast<short>(0x03));
}

// ═══════════════════════════════════════════════════════════════════════════
// toOrderBy – all explicit case branches + default fallback
// ═══════════════════════════════════════════════════════════════════════════

void TestMusicProperties::testToOrderBy_validValues()
{
    // Branch: case 0x11 → ASC
    QCOMPARE(toOrderBy(static_cast<short>(0x11)), OrderByEnum::ASC);

    // Branch: case 0x12 → DESC
    QCOMPARE(toOrderBy(static_cast<short>(0x12)), OrderByEnum::DESC);
}

void TestMusicProperties::testToOrderBy_invalidValue_returnsDefault()
{
    // Branch: default → ASC (fallback for any value not in {0x11,0x12})
    QCOMPARE(toOrderBy(static_cast<short>(0x00)), OrderByEnum::ASC);
    QCOMPARE(toOrderBy(static_cast<short>(0x13)), OrderByEnum::ASC);
    QCOMPARE(toOrderBy(static_cast<short>(-1)),   OrderByEnum::ASC);
    QCOMPARE(toOrderBy(static_cast<short>(0xFF)), OrderByEnum::ASC);
}

// ═══════════════════════════════════════════════════════════════════════════
// toShort(OrderByEnum) – all case branches
// ═══════════════════════════════════════════════════════════════════════════

void TestMusicProperties::testToShort_orderBy()
{
    // Branch: ASC  → 0x11
    QCOMPARE(toShort(OrderByEnum::ASC),  static_cast<short>(0x11));

    // Branch: DESC → 0x12
    QCOMPARE(toShort(OrderByEnum::DESC), static_cast<short>(0x12));
}

// ═══════════════════════════════════════════════════════════════════════════
// Round-trip: enum → short → enum must be identity
// ═══════════════════════════════════════════════════════════════════════════

void TestMusicProperties::testSortByRoundtrip()
{
    // toSortBy(toShort(e)) == e for every SortByEnum value
    QCOMPARE(toSortBy(toShort(SortByEnum::TITLE)),  SortByEnum::TITLE);
    QCOMPARE(toSortBy(toShort(SortByEnum::ARTIST)), SortByEnum::ARTIST);
    QCOMPARE(toSortBy(toShort(SortByEnum::ALBUM)),  SortByEnum::ALBUM);

    // Reverse direction: toShort(toSortBy(n)) == n for valid codes
    QCOMPARE(toShort(toSortBy(static_cast<short>(0x01))), static_cast<short>(0x01));
    QCOMPARE(toShort(toSortBy(static_cast<short>(0x02))), static_cast<short>(0x02));
    QCOMPARE(toShort(toSortBy(static_cast<short>(0x03))), static_cast<short>(0x03));
}

void TestMusicProperties::testOrderByRoundtrip()
{
    // toOrderBy(toShort(e)) == e for every OrderByEnum value
    QCOMPARE(toOrderBy(toShort(OrderByEnum::ASC)),  OrderByEnum::ASC);
    QCOMPARE(toOrderBy(toShort(OrderByEnum::DESC)), OrderByEnum::DESC);

    // Reverse direction: toShort(toOrderBy(n)) == n for valid codes
    QCOMPARE(toShort(toOrderBy(static_cast<short>(0x11))), static_cast<short>(0x11));
    QCOMPARE(toShort(toOrderBy(static_cast<short>(0x12))), static_cast<short>(0x12));
}
