#ifndef TESTLYRICIGNORERULE_H
#define TESTLYRICIGNORERULE_H

#include <QApplication>
#include <QTest>
#include "../src/Data/LyricIgnoreRule.h"

using namespace PROPERTIES;

class TestLyricIgnoreRule : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void constructor();
    void copyConstructor();
    void copyAssignment();
    void selfAssignment();
    void equality();
    void inequality();
    void getters();
    void setRulesStr();
    void stringConversions();
    void stringToEnum_defaults();
};

#endif