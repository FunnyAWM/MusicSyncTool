#ifndef TESTMUSICPROPERTIES_H
#define TESTMUSICPROPERTIES_H

#include <QTest>
#include "../src/Core/MusicProperties.h"

using namespace PROPERTIES;

class TestMusicProperties : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void toSortBy_valid();
    void toSortBy_default();
    void toSortBy_boundary();
    void toShort_sortBy();
    void toShort_sortBy_default();
    void toOrderBy_valid();
    void toOrderBy_default();
    void toShort_orderBy();
    void sortBy_roundtrip();
    void orderBy_roundtrip();
};

#endif