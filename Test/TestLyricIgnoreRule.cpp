#include "TestLyricIgnoreRule.h"

void TestLyricIgnoreRule::constructor() {
    LyricIgnoreRule r(RuleType::INCLUDES, RuleField::TITLE, "test");
    QCOMPARE(r.getRuleType(), RuleType::INCLUDES);
    QCOMPARE(r.getRuleField(), RuleField::TITLE);
    QCOMPARE(r.getRuleName(), QString("test"));
    QCOMPARE(r.getRuleTypeStr(), QString(""));
    QCOMPARE(r.getRuleFieldStr(), QString(""));
}

void TestLyricIgnoreRule::copyConstructor() {
    LyricIgnoreRule orig(RuleType::EXCLUDES, RuleField::ARTIST, "orig");
    orig.setRulesStr();
    LyricIgnoreRule cpy(orig);
    QCOMPARE(cpy.getRuleType(), RuleType::EXCLUDES);
    QCOMPARE(cpy.getRuleField(), RuleField::ARTIST);
    QCOMPARE(cpy.getRuleName(), QString("orig"));
    QCOMPARE(cpy.getRuleTypeStr(), orig.getRuleTypeStr());
    QCOMPARE(cpy.getRuleFieldStr(), orig.getRuleFieldStr());
}

void TestLyricIgnoreRule::copyAssignment() {
    LyricIgnoreRule src(RuleType::EXCLUDES, RuleField::ALBUM, "src");
    src.setRulesStr();
    LyricIgnoreRule dst(RuleType::INCLUDES, RuleField::TITLE, "dst");
    dst = src;
    QCOMPARE(dst.getRuleType(), RuleType::EXCLUDES);
    QCOMPARE(dst.getRuleField(), RuleField::ALBUM);
    QCOMPARE(dst.getRuleName(), QString("src"));
}

void TestLyricIgnoreRule::selfAssignment() {
    LyricIgnoreRule r(RuleType::INCLUDES, RuleField::TITLE, "self");
    r = r;
    QCOMPARE(r.getRuleType(), RuleType::INCLUDES);
    QCOMPARE(r.getRuleName(), QString("self"));
}

void TestLyricIgnoreRule::equality() {
    LyricIgnoreRule a(RuleType::INCLUDES, RuleField::TITLE, "same");
    LyricIgnoreRule b(RuleType::INCLUDES, RuleField::TITLE, "same");
    QVERIFY(a == b);
    LyricIgnoreRule c(RuleType::EXCLUDES, RuleField::TITLE, "same");
    QVERIFY(!(a == c));
}

void TestLyricIgnoreRule::inequality() {
    LyricIgnoreRule a(RuleType::INCLUDES, RuleField::TITLE, "r");
    LyricIgnoreRule b(RuleType::INCLUDES, RuleField::TITLE, "r");
    QVERIFY(!(a != b));
    LyricIgnoreRule c(RuleType::EXCLUDES, RuleField::TITLE, "r");
    QVERIFY(a != c);
}

void TestLyricIgnoreRule::getters() {
    LyricIgnoreRule r1(RuleType::INCLUDES, RuleField::TITLE, "x");
    QCOMPARE(r1.getRuleType(), RuleType::INCLUDES);
    LyricIgnoreRule r2(RuleType::EXCLUDES, RuleField::ARTIST, "x");
    QCOMPARE(r2.getRuleField(), RuleField::ARTIST);
    LyricIgnoreRule r3(RuleType::INCLUDES, RuleField::ALBUM, "x");
    QCOMPARE(r3.getRuleField(), RuleField::ALBUM);
}

void TestLyricIgnoreRule::setRulesStr() {
    LyricIgnoreRule r1(RuleType::INCLUDES, RuleField::TITLE, "r1");
    r1.setRulesStr();
    QCOMPARE(r1.getRuleTypeStr(), QString::fromUtf8("\u5305\u542b"));
    QCOMPARE(r1.getRuleFieldStr(), QString::fromUtf8("\u540d\u79f0"));
    LyricIgnoreRule r2(RuleType::EXCLUDES, RuleField::ALBUM, "r2");
    r2.setRulesStr();
    QCOMPARE(r2.getRuleTypeStr(), QString::fromUtf8("\u6392\u9664"));
    QCOMPARE(r2.getRuleFieldStr(), QString::fromUtf8("\u4e13\u8f91"));
}

void TestLyricIgnoreRule::stringConversions() {
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::TITLE), QString::fromUtf8("\u540d\u79f0"));
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::ARTIST), QString::fromUtf8("\u827a\u672f\u5bb6"));
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::ALBUM), QString::fromUtf8("\u4e13\u8f91"));
    QCOMPARE(LyricIgnoreRule::ignoreRulesToString(RuleType::INCLUDES), QString::fromUtf8("\u5305\u542b"));
    QCOMPARE(LyricIgnoreRule::ignoreRulesToString(RuleType::EXCLUDES), QString::fromUtf8("\u6392\u9664"));
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules(QString::fromUtf8("\u5305\u542b")), RuleType::INCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules(QString::fromUtf8("\u6392\u9664")), RuleType::EXCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(QString::fromUtf8("\u540d\u79f0")), RuleField::TITLE);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(QString::fromUtf8("\u827a\u672f\u5bb6")), RuleField::ARTIST);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(QString::fromUtf8("\u4e13\u8f91")), RuleField::ALBUM);
}

void TestLyricIgnoreRule::stringToEnum_defaults() {
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules("unknown"), RuleType::INCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules(""), RuleType::INCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules("unknown"), RuleField::TITLE);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(""), RuleField::TITLE);
}