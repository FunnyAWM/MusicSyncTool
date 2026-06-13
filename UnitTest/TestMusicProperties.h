#ifndef TESTMUSICPROPERTIES_H
#define TESTMUSICPROPERTIES_H

#include <QTest>
#include "../src/Core/MusicProperties.h"

using namespace PROPERTIES;

class TestMusicProperties : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testToSortBy_validValues();
    void testToSortBy_invalidValue_returnsDefault();
    void testToShort_sortBy();
    void testToOrderBy_validValues();
    void testToOrderBy_invalidValue_returnsDefault();
    void testToShort_orderBy();
    void testSortByRoundtrip();
    void testOrderByRoundtrip();
};

#endif // TESTMUSICPROPERTIES_H
