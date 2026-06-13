/**
 * @file TestStringSimilarity.cpp
 * @brief White-box branch coverage tests for the StringSimilarity namespace
 *        (src/Core/StringSimilarity.h + src/Core/StringSimilarity.cpp)
 *
 * Branch coverage targets:
 *   buildKMPNext       : empty / single-char / repeated / no-repeat patterns
 *   kmpSimilarity      : empty-input short-circuit, exact match, case-insensitive
 *                        match, KMP full-match early return, threshold boundary
 *   isCJKChar          : each Unicode range branch (CJK unified, ext A, hiragana,
 *                        katakana, hangul, hangul-ext, fullwidth, ASCII negative)
 *   isCJKString        : early-exit on first CJK char vs full scan with no CJK
 *   fullWidthToHalfWidth: fullwidth-ASCII range, ideographic-space, non-convertible
 *   normalizeCJKString : CJK-passthrough, ASCII-lowercase, space-filter, simplified
 *   cjkStringSimilarity: both-empty, one-empty, Levenshtein DP
 *   fuzzyMatch         : exact-match shortcut, CJK dispatch path, KMP dispatch path
 */

#include "TestStringSimilarity.h"

// ═══════════════════════════════════════════════════════════════════════════
// buildKMPNext – KMP failure function
// ═══════════════════════════════════════════════════════════════════════════

void TestStringSimilarity::testBuildKMPNext_emptyPattern()
{
    // Branch: m == 0 → for-loop body never executes, returns empty vector
    std::vector<int> next = buildKMPNext(QString(""));
    QVERIFY(next.empty());
}

void TestStringSimilarity::testBuildKMPNext_singleChar()
{
    // Branch: m == 1 → for-loop condition (i=1 < m=1) is false immediately
    // j stays 0, next[0] is never written (initialised to 0)
    std::vector<int> next = buildKMPNext(QString("a"));
    QCOMPARE(static_cast<int>(next.size()), 1);
    QCOMPARE(next[0], 0);
}

void TestStringSimilarity::testBuildKMPNext_repeatedPattern()
{
    // "aaaa": each position extends the previous match by 1
    // i=1: j=0, 'a'=='a' → j=1, next[1]=1
    // i=2: j=1, 'a'=='a' → j=2, next[2]=2
    // i=3: j=2, 'a'=='a' → j=3, next[3]=3
    std::vector<int> next = buildKMPNext(QString("aaaa"));
    QCOMPARE(static_cast<int>(next.size()), 4);
    QCOMPARE(next[0], 0);
    QCOMPARE(next[1], 1);
    QCOMPARE(next[2], 2);
    QCOMPARE(next[3], 3);

    // "aabaa": partial prefix-suffix overlap
    // i=1: 'a'=='a' → j=1, next[1]=1
    // i=2: 'b'!='a', j=next[0]=0; 'b'!='a' → j stays 0, next[2]=0
    // i=3: 'a'=='a' → j=1, next[3]=1
    // i=4: 'a'=='a' → j=2, next[4]=2
    std::vector<int> next2 = buildKMPNext(QString("aabaa"));
    QCOMPARE(static_cast<int>(next2.size()), 5);
    QCOMPARE(next2[0], 0);
    QCOMPARE(next2[1], 1);
    QCOMPARE(next2[2], 0);
    QCOMPARE(next2[3], 1);
    QCOMPARE(next2[4], 2);
}

void TestStringSimilarity::testBuildKMPNext_noRepeat()
{
    // "abcd": all characters distinct → no proper prefix equals a suffix
    // For every i: while-loop never fires, if-condition fails, next[i]=0
    std::vector<int> next = buildKMPNext(QString("abcd"));
    QCOMPARE(static_cast<int>(next.size()), 4);
    QCOMPARE(next[0], 0);
    QCOMPARE(next[1], 0);
    QCOMPARE(next[2], 0);
    QCOMPARE(next[3], 0);
}

// ═══════════════════════════════════════════════════════════════════════════
// kmpSimilarity – KMP matching with threshold
// ═══════════════════════════════════════════════════════════════════════════

void TestStringSimilarity::testKmpSimilarity_exactMatch()
{
    // Branch: text == pattern → return true  (exact match short-circuit)
    QVERIFY(kmpSimilarity("hello", "hello"));

    // Branch: lowerText == lowerPattern → return true  (case-insensitive hit)
    QVERIFY(kmpSimilarity("Hello", "hello"));
    QVERIFY(kmpSimilarity("WORLD", "world"));
}

void TestStringSimilarity::testKmpSimilarity_partialMatch_aboveThreshold()
{
    // Trace: text="abcdef", pattern="abcde"
    //   shorter = "abcde" (len 5), longer = "abcdef" (len 6)
    //   KMP matches positions 0-4 completely → matchCount = 5
    //   similarity = 5/5 = 1.0 >= 0.8 → true
    // Also exercises the early-return branch: j == shorter.length() → return true
    QVERIFY(kmpSimilarity("abcdef", "abcde"));

    // Boundary test: similarity exactly at threshold (0.8 >= 0.8 → true)
    // text="abcdx", pattern="abcde"
    //   shorter = "abcde", longer = "abcdx"
    //   Matches a,b,c,d (matchCount=4); 'x' != 'e', j=next[3]=0; loop ends
    //   similarity = 4/5 = 0.8 >= 0.8 → true
    QVERIFY(kmpSimilarity("abcdx", "abcde"));

    // Custom low threshold: similarity = 4/5 = 0.8 >= 0.5 → true
    QVERIFY(kmpSimilarity("abcde", "abcdx", 0.5));
}

void TestStringSimilarity::testKmpSimilarity_noMatch()
{
    // Completely disjoint character sets → matchCount = 0
    // similarity = 0/3 = 0.0 < 0.8 → false
    QVERIFY(!kmpSimilarity("abc", "xyz"));

    // Partial match but below threshold:
    // text="abx", pattern="abc"
    //   shorter = "abc" (len 3), longer = "abx" (len 3)
    //   Matches a,b (matchCount=2); 'x' != 'c', loop ends
    //   similarity = 2/3 ≈ 0.667 < 0.8 → false
    QVERIFY(!kmpSimilarity("abx", "abc"));

    // Single-char mismatch at threshold 0.8:
    // similarity = 0/1 = 0.0 < 0.8 → false
    QVERIFY(!kmpSimilarity("a", "b"));
}

void TestStringSimilarity::testKmpSimilarity_emptyPattern()
{
    // Branch: text.isEmpty() || pattern.isEmpty() is true
    //   Sub-branch: both empty → text.isEmpty() && pattern.isEmpty() is true → true
    QVERIFY(kmpSimilarity("", ""));

    //   Sub-branch: only pattern empty → false
    QVERIFY(!kmpSimilarity("hello", ""));

    //   Sub-branch: only text empty → false
    QVERIFY(!kmpSimilarity("", "hello"));
}

// ═══════════════════════════════════════════════════════════════════════════
// isCJKChar / isCJKString
// ═══════════════════════════════════════════════════════════════════════════

void TestStringSimilarity::testIsCJKChar_chinese()
{
    // Branch: 0x4E00–0x9FFF (CJK Unified Ideographs) → true
    QVERIFY(isCJKChar(QChar(0x4E00)));   // first char in range
    QVERIFY(isCJKChar(QChar(0x9FFF)));   // last char in range
    QVERIFY(isCJKChar(QString::fromUtf8("\u4e2d")[0])); // 中

    // Branch: 0x3400–0x4DBF (CJK Extension A) → true
    QVERIFY(isCJKChar(QChar(0x3400)));
    QVERIFY(isCJKChar(QChar(0x4DBF)));
}

void TestStringSimilarity::testIsCJKChar_japanese()
{
    // Branch: 0x3040–0x309F (Hiragana) → true
    QVERIFY(isCJKChar(QChar(0x3040)));
    QVERIFY(isCJKChar(QChar(0x309F)));

    // Branch: 0x30A0–0x30FF (Katakana) → true
    QVERIFY(isCJKChar(QChar(0x30A0)));
    QVERIFY(isCJKChar(QChar(0x30FF)));
}

void TestStringSimilarity::testIsCJKChar_korean()
{
    // Branch: 0xAC00–0xD7AF (Hangul Syllables) → true
    QVERIFY(isCJKChar(QChar(0xAC00)));
    QVERIFY(isCJKChar(QChar(0xD7AF)));

    // Branch: 0x1100–0x11FF (Hangul Jamo / Korean extension) → true
    QVERIFY(isCJKChar(QChar(0x1100)));
    QVERIFY(isCJKChar(QChar(0x11FF)));
}

void TestStringSimilarity::testIsCJKChar_ascii()
{
    // Branch: none of the CJK ranges match → false
    QVERIFY(!isCJKChar(QChar('A')));       // 0x41
    QVERIFY(!isCJKChar(QChar('z')));       // 0x7A
    QVERIFY(!isCJKChar(QChar('0')));       // 0x30
    QVERIFY(!isCJKChar(QChar(' ')));       // 0x20
    QVERIFY(!isCJKChar(QChar(0x2FFF)));    // just below CJK Extension A
}

void TestStringSimilarity::testIsCJKString_mixed()
{
    // Branch: first CJK char found → return true (early exit from loop)
    QVERIFY(isCJKString(QString::fromUtf8("hello\u4e16\u754c"))); // "hello世界"

    // CJK char at the very start → immediate return true
    QVERIFY(isCJKString(QString::fromUtf8("\u4e2d\u6587")));       // "中文"

    // CJK char at the end only
    QVERIFY(isCJKString(QString("abc") + QString::fromUtf8("\u4e00")));
}

void TestStringSimilarity::testIsCJKString_pureAscii()
{
    // Branch: loop completes without finding any CJK char → return false
    QVERIFY(!isCJKString("hello world"));
    QVERIFY(!isCJKString("ABC123"));
    QVERIFY(!isCJKString("")); // empty string → loop body never runs → false
}

// ═══════════════════════════════════════════════════════════════════════════
// fullWidthToHalfWidth / normalizeCJKString
// ═══════════════════════════════════════════════════════════════════════════

void TestStringSimilarity::testFullWidthToHalfWidth()
{
    // Branch: 0xFF01–0xFF5E (fullwidth ASCII) → QChar(unicode - 0xFEE0)
    QCOMPARE(fullWidthToHalfWidth(QChar(0xFF21)), QChar('A'));   // Ａ → A
    QCOMPARE(fullWidthToHalfWidth(QChar(0xFF41)), QChar('a'));   // ａ → a
    QCOMPARE(fullWidthToHalfWidth(QChar(0xFF10)), QChar('0'));   // ０ → 0

    // Branch: unicode == 0x3000 (ideographic space) → QChar(0x0020)
    QCOMPARE(fullWidthToHalfWidth(QChar(0x3000)), QChar(' '));   // → space

    // Branch: default (not in any conversion range) → return ch unchanged
    QCOMPARE(fullWidthToHalfWidth(QChar('A')), QChar('A'));
    QCOMPARE(fullWidthToHalfWidth(QChar('z')), QChar('z'));

    // CJK characters pass through unchanged (no branch handles them)
    QChar cjk(QString::fromUtf8("\u4e2d")[0]); // 中
    QCOMPARE(fullWidthToHalfWidth(cjk), cjk);
}

void TestStringSimilarity::testNormalizeCJKString()
{
    // Mixed CJK + fullwidth + ASCII
    //   Ａ (0xFF21) → fullWidthToHalfWidth → 'A' → isCJKChar? No → toLower → 'a'
    //   中           → fullWidthToHalfWidth → 中  → isCJKChar? Yes → keep
    //   Ｂ (0xFF22) → fullWidthToHalfWidth → 'B' → isCJKChar? No → toLower → 'b'
    //   Result before simplified(): "a中b"
    //   simplified() → "a中b" (no extra whitespace to collapse)
    QString input = QString::fromUtf8("\uFF21\u4E2D\uFF22"); // Ａ中Ｂ
    QCOMPARE(normalizeCJKString(input), QString::fromUtf8("a\u4E2Db")); // a中b

    // Space handling: ideographic space → regular space, then simplified() trims
    //   0x3000 → ' ', isSpace && == ' ' → kept
    //   hello (ASCII) → toLower → hello
    //   0x3000 → ' '
    //   WORLD → toLower → world
    //   Before simplified: "hello world", after simplified: "hello world"
    QString spaced = QString::fromUtf8("hello\u3000WORLD");
    QCOMPARE(normalizeCJKString(spaced), QString("hello world"));

    // Pure ASCII with case folding
    QCOMPARE(normalizeCJKString("ABC"), QString("abc"));

    // Empty string → empty result
    QCOMPARE(normalizeCJKString(""), QString(""));

    // Tab character: isSpace && != ' ' → filtered out
    //   "a\tb" → 'a' kept, '\t' filtered, 'b' kept → "ab"
    QCOMPARE(normalizeCJKString("a\tb"), QString("ab"));
}

// ═══════════════════════════════════════════════════════════════════════════
// cjkStringSimilarity – Levenshtein-based
// ═══════════════════════════════════════════════════════════════════════════

void TestStringSimilarity::testCjkStringSimilarity_identical()
{
    // Branch: both empty → return 1.0
    QCOMPARE(cjkStringSimilarity("", ""), 1.0);

    // Non-empty identical strings → editDistance = 0, similarity = 1.0
    QCOMPARE(cjkStringSimilarity(
        QString::fromUtf8("\u4f60\u597d"),   // 你好
        QString::fromUtf8("\u4f60\u597d")), 1.0);

    QCOMPARE(cjkStringSimilarity("abc", "abc"), 1.0);
}

void TestStringSimilarity::testCjkStringSimilarity_completelyDifferent()
{
    // Single chars that differ → editDistance=1, maxLen=1, sim = 1-1/1 = 0.0
    QCOMPARE(cjkStringSimilarity(
        QString::fromUtf8("\u4f60"),   // 你
        QString::fromUtf8("\u6211")), 0.0); // 我

    // Equal-length all-different: "abc" vs "xyz"
    // editDistance = 3 (all substitutions), maxLen = 3, sim = 0.0
    QCOMPARE(cjkStringSimilarity("abc", "xyz"), 0.0);
}

void TestStringSimilarity::testCjkStringSimilarity_similar()
{
    // "abc" vs "ab" → editDistance=1, maxLen=3, sim = 1 - 1/3 ≈ 0.6667
    double sim = cjkStringSimilarity("abc", "ab");
    QVERIFY(sim > 0.6);
    QVERIFY(sim < 0.7);

    // "abcd" vs "abce" → editDistance=1, maxLen=4, sim = 1 - 1/4 = 0.75
    QCOMPARE(cjkStringSimilarity("abcd", "abce"), 0.75);

    // CJK strings differing by one char: "你好世" vs "你好呀"
    // editDistance=1, maxLen=3, sim = 1 - 1/3 ≈ 0.6667
    double cjkSim = cjkStringSimilarity(
        QString::fromUtf8("\u4f60\u597d\u4e16"), // 你好世
        QString::fromUtf8("\u4f60\u597d\u5440")); // 你好呀
    QVERIFY(cjkSim > 0.6);
    QVERIFY(cjkSim < 0.7);
}

void TestStringSimilarity::testCjkStringSimilarity_empty()
{
    // Branch: both empty → return 1.0
    QCOMPARE(cjkStringSimilarity("", ""), 1.0);

    // Branch: one empty, other non-empty → return 0.0
    QCOMPARE(cjkStringSimilarity("", "abc"), 0.0);
    QCOMPARE(cjkStringSimilarity("abc", ""), 0.0);

    // CJK: one empty → 0.0
    QCOMPARE(cjkStringSimilarity("", QString::fromUtf8("\u4f60")), 0.0);
}

// ═══════════════════════════════════════════════════════════════════════════
// fuzzyMatch – top-level dispatch (CJK path vs KMP path)
// ═══════════════════════════════════════════════════════════════════════════

void TestStringSimilarity::testFuzzyMatch_cjkStrings_similar()
{
    // Branch: str1 == str2 → return true  (exact match shortcut)
    QVERIFY(fuzzyMatch(
        QString::fromUtf8("\u4f60\u597d"),   // 你好
        QString::fromUtf8("\u4f60\u597d"))); // 你好

    // Branch: CJK path → normalize → norm1 == norm2 → return true
    // Fullwidth Ａ normalises to 'a'; both become identical after normalisation
    QString s1 = QString::fromUtf8("\uFF21\u4E2D"); // Ａ中
    QString s2 = QString::fromUtf8("a\u4E2D");       // a中
    QVERIFY(fuzzyMatch(s1, s2));
}

void TestStringSimilarity::testFuzzyMatch_cjkStrings_different()
{
    // CJK path: normalised strings differ, cjkStringSimilarity < 0.85 → false
    // "你好" vs "我们" → editDistance=2, maxLen=2, sim=0.0 < 0.85
    QVERIFY(!fuzzyMatch(
        QString::fromUtf8("\u4f60\u597d"),   // 你好
        QString::fromUtf8("\u6211\u4eec"))); // 我们

    // CJK path: similarity above 0.85 but not identical
    // Need strings with similarity >= 0.85:
    // "你好世界你好世界" (8 chars) vs "你好世界你好世x" (8 chars, 1 diff)
    // editDistance=1, maxLen=8, sim=0.875 >= 0.85 → true
    QVERIFY(fuzzyMatch(
        QString::fromUtf8("\u4f60\u597d\u4e16\u754c\u4f60\u597d\u4e16\u754c"), // 你好世界你好世界
        QString::fromUtf8("\u4f60\u597d\u4e16\u754c\u4f60\u597d\u4e16x")));    // 你好世界你好世x
}

void TestStringSimilarity::testFuzzyMatch_asciiStrings_similar()
{
    // Branch: str1 == str2 → return true  (exact match shortcut)
    QVERIFY(fuzzyMatch("hello world", "hello world"));

    // Branch: no CJK → KMP path → case-insensitive match
    QVERIFY(fuzzyMatch("Hello World", "hello world"));

    // Branch: no CJK → KMP path → partial match at threshold boundary
    // "abcdx" vs "abcde" → similarity = 4/5 = 0.8, but float threshold (0.8f)
    // is slightly greater than double 0.8 due to precision, so this returns false
    QVERIFY(!fuzzyMatch("abcdx", "abcde"));

    // Use wider margin to get a clear pass: "abcdef" vs "abcde" → 5/6 ≈ 0.833
    QVERIFY(fuzzyMatch("abcdef", "abcde"));
}

void TestStringSimilarity::testFuzzyMatch_asciiStrings_different()
{
    // Branch: no CJK → KMP path → similarity below threshold → false
    // "abx" vs "abc" → similarity = 2/3 ≈ 0.667 < 0.8 → false
    QVERIFY(!fuzzyMatch("abx", "abc"));

    // Completely different strings → false
    QVERIFY(!fuzzyMatch("hello", "world"));

    // Different lengths, no common prefix → false
    QVERIFY(!fuzzyMatch("a", "xyz"));
}
