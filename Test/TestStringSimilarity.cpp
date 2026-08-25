#include "TestStringSimilarity.h"

void TestStringSimilarity::buildKMPNext_empty() {
    std::vector<int> next = buildKMPNext("");
    QVERIFY(next.empty());
}

void TestStringSimilarity::buildKMPNext_single() {
    std::vector<int> next = buildKMPNext("a");
    QCOMPARE((int)next.size(), 1);
    QCOMPARE(next[0], 0);
}

void TestStringSimilarity::buildKMPNext_repeated() {
    std::vector<int> next = buildKMPNext("aaaa");
    QCOMPARE((int)next.size(), 4);
    QCOMPARE(next[0], 0); QCOMPARE(next[1], 1);
    QCOMPARE(next[2], 2); QCOMPARE(next[3], 3);
}

void TestStringSimilarity::buildKMPNext_noRepeat() {
    std::vector<int> next = buildKMPNext("abcd");
    for (int i = 0; i < 4; ++i) QCOMPARE(next[i], 0);
}

void TestStringSimilarity::kmpSimilarity_exact() {
    QVERIFY(kmpSimilarity("hello", "hello"));
    QVERIFY(kmpSimilarity("abcdef", "abcde"));
}

void TestStringSimilarity::kmpSimilarity_caseInsensitive() {
    QVERIFY(kmpSimilarity("Hello", "hello"));
    QVERIFY(kmpSimilarity("WORLD", "world"));
}

void TestStringSimilarity::kmpSimilarity_partial() {
    QVERIFY(!kmpSimilarity("abcdx", "abcde"));
    QVERIFY(kmpSimilarity("abcdx", "abcde", 0.8));
    QVERIFY(kmpSimilarity("abcde", "abcdx", 0.5));
}

void TestStringSimilarity::kmpSimilarity_noMatch() {
    QVERIFY(!kmpSimilarity("abc", "xyz"));
    QVERIFY(!kmpSimilarity("a", "b"));
}

void TestStringSimilarity::kmpSimilarity_empty() {
    QVERIFY(kmpSimilarity("", ""));
    QVERIFY(!kmpSimilarity("hello", ""));
    QVERIFY(!kmpSimilarity("", "hello"));
}

void TestStringSimilarity::isCJKChar_ranges() {
    QVERIFY(isCJKChar(QChar(0x4E00)));
    QVERIFY(isCJKChar(QChar(0x3400)));
    QVERIFY(isCJKChar(QChar(0x3040)));
    QVERIFY(isCJKChar(QChar(0x30A0)));
    QVERIFY(isCJKChar(QChar(0xAC00)));
    QVERIFY(isCJKChar(QChar(0x1100)));
    QVERIFY(isCJKChar(QChar(0xFF01)));
}

void TestStringSimilarity::isCJKChar_ascii() {
    QVERIFY(!isCJKChar(QChar('A')));
    QVERIFY(!isCJKChar(QChar('0')));
}

void TestStringSimilarity::isCJKString_mixed() {
    QVERIFY(isCJKString(QString::fromUtf8("hello\u4e16\u754c")));
    QVERIFY(isCJKString(QString::fromUtf8("\u4e2d\u6587")));
}

void TestStringSimilarity::isCJKString_pure() {
    QVERIFY(!isCJKString("hello world"));
    QVERIFY(!isCJKString(""));
}

void TestStringSimilarity::fullWidthToHalfWidth_convert() {
    QCOMPARE(fullWidthToHalfWidth(QChar(0xFF21)), QChar('A'));
    QCOMPARE(fullWidthToHalfWidth(QChar(0x3000)), QChar(' '));
    QCOMPARE(fullWidthToHalfWidth(QChar('A')), QChar('A'));
}

void TestStringSimilarity::normalizeCJKString_mixed() {
    QString input = QString::fromUtf8("\uFF21\u4E2D\uFF22");
    QCOMPARE(normalizeCJKString(input), QString::fromUtf8("a\u4E2Db"));
    QCOMPARE(normalizeCJKString("ABC"), QString("abc"));
}

void TestStringSimilarity::normalizeCJKString_spaces() {
    QCOMPARE(normalizeCJKString("a\tb"), QString("ab"));
    QCOMPARE(normalizeCJKString("  hello  world  "), QString("hello world"));
}

void TestStringSimilarity::cjkStringSimilarity_identical() {
    QCOMPARE(cjkStringSimilarity("", ""), 1.0);
    QCOMPARE(cjkStringSimilarity("abc", "abc"), 1.0);
}

void TestStringSimilarity::cjkStringSimilarity_different() {
    QCOMPARE(cjkStringSimilarity("abc", "xyz"), 0.0);
}

void TestStringSimilarity::cjkStringSimilarity_similar() {
    double s = cjkStringSimilarity("abc", "ab");
    QVERIFY(s > 0.6 && s < 0.7);
    QCOMPARE(cjkStringSimilarity("abcd", "abce"), 0.75);
}

void TestStringSimilarity::cjkStringSimilarity_empty() {
    QCOMPARE(cjkStringSimilarity("", "abc"), 0.0);
    QCOMPARE(cjkStringSimilarity("abc", ""), 0.0);
}

void TestStringSimilarity::fuzzyMatch_cjk() {
    QVERIFY(fuzzyMatch(QString::fromUtf8("\u4f60\u597d"), QString::fromUtf8("\u4f60\u597d")));
    QVERIFY(!fuzzyMatch(QString::fromUtf8("\u4f60\u597d"), QString::fromUtf8("\u6211\u4eec")));
}

void TestStringSimilarity::fuzzyMatch_ascii() {
    QVERIFY(fuzzyMatch("hello world", "hello world"));
    QVERIFY(fuzzyMatch("Hello World", "hello world"));
    QVERIFY(!fuzzyMatch("hello", "world"));
}

void TestStringSimilarity::fuzzyMatch_edgeCases() {
    QVERIFY(fuzzyMatch("abcdef", "abcde"));
    QVERIFY(!fuzzyMatch("abx", "abc"));
}