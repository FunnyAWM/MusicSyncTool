#ifndef TESTQUERYITEM_H
#define TESTQUERYITEM_H

#include <QTest>
#include "../src/Data/QueryItem.h"

class TestQueryItem : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testDefaultConstructor();
    void testFullConstructor();
    void testSetGetTitle();
    void testSetGetArtist();
    void testSetGetAlbum();
    void testSetGetGenre();
    void testSetGetYear();
    void testSetGetTrack();
    void testSetGetFileName();
    void testGetSetSimilarityThreshold();
    void testDefaultSimilarityThreshold();
    void testEquality_identicalItems();
    void testEquality_differentTitles();
    void testEquality_similarTitles_differentArtists();
    void testEquality_withSimilarityThresholdChange();
    void testEquality_emptyItems();
};

#endif // TESTQUERYITEM_H
