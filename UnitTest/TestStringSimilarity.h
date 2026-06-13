#ifndef TESTSTRINGSIMILARITY_H
#define TESTSTRINGSIMILARITY_H

#include <QChar>
#include <QTest>
#include <vector>
#include "../src/Core/StringSimilarity.h"

using namespace StringSimilarity;

class TestStringSimilarity : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testBuildKMPNext_emptyPattern();
    void testBuildKMPNext_singleChar();
    void testBuildKMPNext_repeatedPattern();
    void testBuildKMPNext_noRepeat();
    void testKmpSimilarity_exactMatch();
    void testKmpSimilarity_partialMatch_aboveThreshold();
    void testKmpSimilarity_noMatch();
    void testKmpSimilarity_emptyPattern();
    void testIsCJKChar_chinese();
    void testIsCJKChar_japanese();
    void testIsCJKChar_korean();
    void testIsCJKChar_ascii();
    void testIsCJKString_mixed();
    void testIsCJKString_pureAscii();
    void testFullWidthToHalfWidth();
    void testNormalizeCJKString();
    void testCjkStringSimilarity_identical();
    void testCjkStringSimilarity_completelyDifferent();
    void testCjkStringSimilarity_similar();
    void testCjkStringSimilarity_empty();
    void testFuzzyMatch_cjkStrings_similar();
    void testFuzzyMatch_cjkStrings_different();
    void testFuzzyMatch_asciiStrings_similar();
    void testFuzzyMatch_asciiStrings_different();
};

#endif // TESTSTRINGSIMILARITY_H
