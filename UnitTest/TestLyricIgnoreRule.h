#ifndef TESTLYRICIGNORERULE_H
#define TESTLYRICIGNORERULE_H

#include <QApplication>
#include <QTest>
#include "../src/Data/LyricIgnoreRule.h"

using namespace PROPERTIES;

class TestLyricIgnoreRule : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testPrimaryConstructor();
    void testCopyConstructor();
    void testCopyAssignment();
    void testEquality_sameRules();
    void testEquality_differentRuleType();
    void testEquality_differentRuleField();
    void testEquality_differentRuleName();
    void testInequalityOperator();
    void testGetRuleType();
    void testGetRuleField();
    void testGetRuleName();
    void testGetRuleTypeStr();
    void testGetRuleFieldStr();
    void testSetRulesStr();
    void testLyricRulesToString();
    void testIgnoreRulesToString();
    void testStringToIgnoreRules();
    void testStringToLyricRules();
    void testStringToIgnoreRules_invalidDefault();
    void testStringToLyricRules_invalidDefault();
};

#endif // TESTLYRICIGNORERULE_H
