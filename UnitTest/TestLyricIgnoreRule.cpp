/**
 * @file TestLyricIgnoreRule.cpp
 * @brief White-box branch coverage tests for the LyricIgnoreRule class
 *        (src/Data/LyricIgnoreRule.h + src/Data/LyricIgnoreRule.cpp)
 *
 * Branch coverage targets:
 *   Primary constructor    : member initialisation, ruleTypeStr/ruleFieldStr NOT set
 *   Copy constructor       : copies enum+name members, does NOT copy string members
 *   Copy assignment        : self-assignment guard + full member copy
 *   operator==             : all three fields compared (inline)
 *   operator!=             : negation of == (inline)
 *   Getters                : getRuleType / getRuleField / getRuleName
 *   lyricRulesToString     : TITLE / ARTIST / ALBUM / default switch branches
 *   ignoreRulesToString    : INCLUDES / EXCLUDES / default switch branches
 *   setRulesStr            : INCLUDES/EXCLUDES + TITLE/ARTIST/ALBUM switch branches
 *   getRuleTypeStr         : returns stored string (empty until setRulesStr called)
 *   getRuleFieldStr        : returns stored string (empty until setRulesStr called)
 *   stringToIgnoreRules    : "包含" / "排除" / default branches
 *   stringToLyricRules     : "名称" / "艺术家" / "专辑" / default branches
 *
 * NOTE: lyricRulesToString and ignoreRulesToString use tr(), which requires
 *       a QApplication instance (provided by the test main.cpp).
 */

#include "TestLyricIgnoreRule.h"

// ═══════════════════════════════════════════════════════════════════════════
// Constructors
// ═══════════════════════════════════════════════════════════════════════════

void TestLyricIgnoreRule::testPrimaryConstructor()
{
    LyricIgnoreRule rule(RuleType::INCLUDES, RuleField::TITLE,
                         QString::fromUtf8("test_rule"));

    QCOMPARE(rule.getRuleType(),  RuleType::INCLUDES);
    QCOMPARE(rule.getRuleField(), RuleField::TITLE);
    QCOMPARE(rule.getRuleName(),  QString::fromUtf8("test_rule"));

    // Primary constructor does NOT initialise ruleTypeStr or ruleFieldStr
    QCOMPARE(rule.getRuleTypeStr(), QString(""));
    QCOMPARE(rule.getRuleFieldStr(), QString(""));
}

void TestLyricIgnoreRule::testCopyConstructor()
{
    LyricIgnoreRule original(RuleType::EXCLUDES, RuleField::ARTIST,
                             QString::fromUtf8("original_name"));

    // Populate the string members on the original
    original.setRulesStr();
    QCOMPARE(original.getRuleTypeStr(), QString::fromUtf8("\u6392\u9664")); // 排除
    QCOMPARE(original.getRuleFieldStr(), QString::fromUtf8("\u827a\u672f\u5bb6")); // 艺术家

    LyricIgnoreRule copy(original);

    // All members (including string representations) are copied
    QCOMPARE(copy.getRuleType(),  RuleType::EXCLUDES);
    QCOMPARE(copy.getRuleField(), RuleField::ARTIST);
    QCOMPARE(copy.getRuleName(),  QString::fromUtf8("original_name"));
    QCOMPARE(copy.getRuleTypeStr(), QString::fromUtf8("\u6392\u9664"));       // 排除
    QCOMPARE(copy.getRuleFieldStr(), QString::fromUtf8("\u827a\u672f\u5bb6")); // 艺术家
}

void TestLyricIgnoreRule::testCopyAssignment()
{
    LyricIgnoreRule source(RuleType::EXCLUDES, RuleField::ALBUM,
                           QString::fromUtf8("source_name"));
    source.setRulesStr();

    LyricIgnoreRule target(RuleType::INCLUDES, RuleField::TITLE,
                           QString::fromUtf8("target_name"));

    target = source;

    // All members (including string representations) are copied
    QCOMPARE(target.getRuleType(),     RuleType::EXCLUDES);
    QCOMPARE(target.getRuleField(),    RuleField::ALBUM);
    QCOMPARE(target.getRuleName(),     QString::fromUtf8("source_name"));
    QCOMPARE(target.getRuleTypeStr(),  QString::fromUtf8("\u6392\u9664"));       // 排除
    QCOMPARE(target.getRuleFieldStr(), QString::fromUtf8("\u4e13\u8f91"));       // 专辑

    // Self-assignment guard: assigning to self must be a safe no-op
    LyricIgnoreRule selfRule(RuleType::INCLUDES, RuleField::TITLE,
                             QString::fromUtf8("self"));
    selfRule = selfRule;
    QCOMPARE(selfRule.getRuleType(),  RuleType::INCLUDES);
    QCOMPARE(selfRule.getRuleField(), RuleField::TITLE);
    QCOMPARE(selfRule.getRuleName(),  QString::fromUtf8("self"));
}

// ═══════════════════════════════════════════════════════════════════════════
// Equality operators
// ═══════════════════════════════════════════════════════════════════════════

void TestLyricIgnoreRule::testEquality_sameRules()
{
    LyricIgnoreRule a(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("same"));
    LyricIgnoreRule b(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("same"));
    QVERIFY(a == b);
}

void TestLyricIgnoreRule::testEquality_differentRuleType()
{
    LyricIgnoreRule a(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("same"));
    LyricIgnoreRule b(RuleType::EXCLUDES, RuleField::TITLE,
                      QString::fromUtf8("same"));
    QVERIFY(!(a == b));
}

void TestLyricIgnoreRule::testEquality_differentRuleField()
{
    LyricIgnoreRule a(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("same"));
    LyricIgnoreRule b(RuleType::INCLUDES, RuleField::ARTIST,
                      QString::fromUtf8("same"));
    QVERIFY(!(a == b));
}

void TestLyricIgnoreRule::testEquality_differentRuleName()
{
    LyricIgnoreRule a(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("name_a"));
    LyricIgnoreRule b(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("name_b"));
    QVERIFY(!(a == b));
}

void TestLyricIgnoreRule::testInequalityOperator()
{
    LyricIgnoreRule a(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("rule"));
    LyricIgnoreRule b(RuleType::INCLUDES, RuleField::TITLE,
                      QString::fromUtf8("rule"));
    LyricIgnoreRule c(RuleType::EXCLUDES, RuleField::TITLE,
                      QString::fromUtf8("rule"));

    // operator!= is the negation of operator==
    QVERIFY(!(a != b)); // equal → != is false
    QVERIFY(a != c);    // different → != is true
}

// ═══════════════════════════════════════════════════════════════════════════
// Getters
// ═══════════════════════════════════════════════════════════════════════════

void TestLyricIgnoreRule::testGetRuleType()
{
    LyricIgnoreRule includesRule(RuleType::INCLUDES, RuleField::TITLE, "x");
    QCOMPARE(includesRule.getRuleType(), RuleType::INCLUDES);

    LyricIgnoreRule excludesRule(RuleType::EXCLUDES, RuleField::TITLE, "x");
    QCOMPARE(excludesRule.getRuleType(), RuleType::EXCLUDES);
}

void TestLyricIgnoreRule::testGetRuleField()
{
    LyricIgnoreRule titleRule(RuleType::INCLUDES, RuleField::TITLE, "x");
    QCOMPARE(titleRule.getRuleField(), RuleField::TITLE);

    LyricIgnoreRule artistRule(RuleType::INCLUDES, RuleField::ARTIST, "x");
    QCOMPARE(artistRule.getRuleField(), RuleField::ARTIST);

    LyricIgnoreRule albumRule(RuleType::INCLUDES, RuleField::ALBUM, "x");
    QCOMPARE(albumRule.getRuleField(), RuleField::ALBUM);
}

void TestLyricIgnoreRule::testGetRuleName()
{
    LyricIgnoreRule rule(RuleType::INCLUDES, RuleField::TITLE,
                         QString::fromUtf8("my_rule_name"));
    QCOMPARE(rule.getRuleName(), QString::fromUtf8("my_rule_name"));

    // Empty name
    LyricIgnoreRule emptyNameRule(RuleType::INCLUDES, RuleField::TITLE, "");
    QCOMPARE(emptyNameRule.getRuleName(), QString(""));
}

// ═══════════════════════════════════════════════════════════════════════════
// String conversions (instance methods using setRulesStr)
// ═══════════════════════════════════════════════════════════════════════════

void TestLyricIgnoreRule::testGetRuleTypeStr()
{
    // Before setRulesStr() is called, the string is empty
    LyricIgnoreRule rule(RuleType::INCLUDES, RuleField::TITLE, "x");
    QCOMPARE(rule.getRuleTypeStr(), QString(""));

    // After setRulesStr(), the string reflects the enum value
    rule.setRulesStr();
    QCOMPARE(rule.getRuleTypeStr(), QString::fromUtf8("\u5305\u542b")); // 包含
}

void TestLyricIgnoreRule::testGetRuleFieldStr()
{
    // Before setRulesStr() is called, the string is empty
    LyricIgnoreRule rule(RuleType::INCLUDES, RuleField::TITLE, "x");
    QCOMPARE(rule.getRuleFieldStr(), QString(""));

    // After setRulesStr(), the string reflects the enum value
    rule.setRulesStr();
    QCOMPARE(rule.getRuleFieldStr(), QString::fromUtf8("\u540d\u79f0")); // 名称
}

void TestLyricIgnoreRule::testSetRulesStr()
{
    // ── Branch: RuleType::INCLUDES → ruleTypeStr = "包含" ──
    LyricIgnoreRule r1(RuleType::INCLUDES, RuleField::TITLE, "r1");
    r1.setRulesStr();
    QCOMPARE(r1.getRuleTypeStr(),  QString::fromUtf8("\u5305\u542b"));         // 包含
    QCOMPARE(r1.getRuleFieldStr(), QString::fromUtf8("\u540d\u79f0"));         // 名称

    // ── Branch: RuleType::EXCLUDES → ruleTypeStr = "排除" ──
    LyricIgnoreRule r2(RuleType::EXCLUDES, RuleField::ARTIST, "r2");
    r2.setRulesStr();
    QCOMPARE(r2.getRuleTypeStr(),  QString::fromUtf8("\u6392\u9664"));         // 排除
    QCOMPARE(r2.getRuleFieldStr(), QString::fromUtf8("\u827a\u672f\u5bb6"));   // 艺术家

    // ── Branch: RuleField::ALBUM → ruleFieldStr = "专辑" ──
    LyricIgnoreRule r3(RuleType::INCLUDES, RuleField::ALBUM, "r3");
    r3.setRulesStr();
    QCOMPARE(r3.getRuleTypeStr(),  QString::fromUtf8("\u5305\u542b"));         // 包含
    QCOMPARE(r3.getRuleFieldStr(), QString::fromUtf8("\u4e13\u8f91"));         // 专辑

    // ── Branch: EXCLUDES + ALBUM ──
    LyricIgnoreRule r4(RuleType::EXCLUDES, RuleField::ALBUM, "r4");
    r4.setRulesStr();
    QCOMPARE(r4.getRuleTypeStr(),  QString::fromUtf8("\u6392\u9664"));         // 排除
    QCOMPARE(r4.getRuleFieldStr(), QString::fromUtf8("\u4e13\u8f91"));         // 专辑
}

// ═══════════════════════════════════════════════════════════════════════════
// Static conversions (lyricRulesToString / ignoreRulesToString use tr())
// ═══════════════════════════════════════════════════════════════════════════

void TestLyricIgnoreRule::testLyricRulesToString()
{
    // Branch: RuleField::TITLE  → tr("名称")
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::TITLE),
             QString::fromUtf8("\u540d\u79f0"));                               // 名称

    // Branch: RuleField::ARTIST → tr("艺术家")
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::ARTIST),
             QString::fromUtf8("\u827a\u672f\u5bb6"));                         // 艺术家

    // Branch: RuleField::ALBUM  → tr("专辑")
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::ALBUM),
             QString::fromUtf8("\u4e13\u8f91"));                               // 专辑
}

void TestLyricIgnoreRule::testIgnoreRulesToString()
{
    // Branch: RuleType::INCLUDES → tr("包含")
    QCOMPARE(LyricIgnoreRule::ignoreRulesToString(RuleType::INCLUDES),
             QString::fromUtf8("\u5305\u542b"));                               // 包含

    // Branch: RuleType::EXCLUDES → tr("排除")
    QCOMPARE(LyricIgnoreRule::ignoreRulesToString(RuleType::EXCLUDES),
             QString::fromUtf8("\u6392\u9664"));                               // 排除
}

void TestLyricIgnoreRule::testStringToIgnoreRules()
{
    // Branch: rule == "包含" → RuleType::INCLUDES
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules(
                 QString::fromUtf8("\u5305\u542b")),                            // 包含
             RuleType::INCLUDES);

    // Branch: rule == "排除" → RuleType::EXCLUDES
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules(
                 QString::fromUtf8("\u6392\u9664")),                            // 排除
             RuleType::EXCLUDES);
}

void TestLyricIgnoreRule::testStringToLyricRules()
{
    // Branch: rule == "名称" → RuleField::TITLE
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(
                 QString::fromUtf8("\u540d\u79f0")),                            // 名称
             RuleField::TITLE);

    // Branch: rule == "艺术家" → RuleField::ARTIST
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(
                 QString::fromUtf8("\u827a\u672f\u5bb6")),                      // 艺术家
             RuleField::ARTIST);

    // Branch: rule == "专辑" → RuleField::ALBUM
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(
                 QString::fromUtf8("\u4e13\u8f91")),                            // 专辑
             RuleField::ALBUM);
}

void TestLyricIgnoreRule::testStringToIgnoreRules_invalidDefault()
{
    // Branch: default → RuleType::INCLUDES (fallback for unrecognised strings)
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules("unknown"),
             RuleType::INCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules(""),
             RuleType::INCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules("includes"),
             RuleType::INCLUDES);
}

void TestLyricIgnoreRule::testStringToLyricRules_invalidDefault()
{
    // Branch: default → RuleField::TITLE (fallback for unrecognised strings)
    QCOMPARE(LyricIgnoreRule::stringToLyricRules("unknown"),
             RuleField::TITLE);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(""),
             RuleField::TITLE);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules("title"),
             RuleField::TITLE);
}
