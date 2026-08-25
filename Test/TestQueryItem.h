#ifndef TESTQUERYITEM_H
#define TESTQUERYITEM_H

#include <QTest>
#include "../src/Data/QueryItem.h"

class TestQueryItem : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void defaultConstructor();
    void fullConstructor();
    void settersAndGetters();
    void similarityThreshold_default();
    void similarityThreshold_set();
    void equality_identical();
    void equality_different();
    void equality_thresholdBoundary();
    void equality_noCoreMatch();
    void equality_empty();
};

#endif