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
    void buildKMPNext_empty();
    void buildKMPNext_single();
    void buildKMPNext_repeated();
    void buildKMPNext_noRepeat();
    void kmpSimilarity_exact();
    void kmpSimilarity_caseInsensitive();
    void kmpSimilarity_partial();
    void kmpSimilarity_noMatch();
    void kmpSimilarity_empty();
    void isCJKChar_ranges();
    void isCJKChar_ascii();
    void isCJKString_mixed();
    void isCJKString_pure();
    void fullWidthToHalfWidth_convert();
    void normalizeCJKString_mixed();
    void normalizeCJKString_spaces();
    void cjkStringSimilarity_identical();
    void cjkStringSimilarity_different();
    void cjkStringSimilarity_similar();
    void cjkStringSimilarity_empty();
    void fuzzyMatch_cjk();
    void fuzzyMatch_ascii();
    void fuzzyMatch_edgeCases();
};

#endif