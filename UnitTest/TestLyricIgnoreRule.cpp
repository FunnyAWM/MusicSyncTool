#include "TestLyricIgnoreRule.h"

void TestLyricIgnoreRule::testConstructor()
{
    // 测试包含规则构造
    LyricIgnoreRule includeRule(RuleType::INCLUDES, RuleField::TITLE, "测试歌曲");
    QCOMPARE(includeRule.getRuleType(), RuleType::INCLUDES);
    QCOMPARE(includeRule.getRuleField(), RuleField::TITLE);
    QCOMPARE(includeRule.getRuleName(), QString("测试歌曲"));
    
    // 测试排除规则构造
    LyricIgnoreRule excludeRule(RuleType::EXCLUDES, RuleField::ARTIST, "测试歌手");
    QCOMPARE(excludeRule.getRuleType(), RuleType::EXCLUDES);
    QCOMPARE(excludeRule.getRuleField(), RuleField::ARTIST);
    QCOMPARE(excludeRule.getRuleName(), QString("测试歌手"));
    
    // 测试专辑字段规则
    LyricIgnoreRule albumRule(RuleType::INCLUDES, RuleField::ALBUM, "测试专辑");
    QCOMPARE(albumRule.getRuleType(), RuleType::INCLUDES);
    QCOMPARE(albumRule.getRuleField(), RuleField::ALBUM);
    QCOMPARE(albumRule.getRuleName(), QString("测试专辑"));
}

void TestLyricIgnoreRule::testCopyConstructor()
{
    LyricIgnoreRule original(RuleType::INCLUDES, RuleField::TITLE, "原始规则");
    LyricIgnoreRule copy(original);
    
    QCOMPARE(copy.getRuleType(), original.getRuleType());
    QCOMPARE(copy.getRuleField(), original.getRuleField());
    QCOMPARE(copy.getRuleName(), original.getRuleName());
    QVERIFY(copy == original);
}

void TestLyricIgnoreRule::testAssignmentOperator()
{
    LyricIgnoreRule rule1(RuleType::INCLUDES, RuleField::TITLE, "规则1");
    LyricIgnoreRule rule2(RuleType::EXCLUDES, RuleField::ARTIST, "规则2");
    
    // 验证初始状态不同
    QVERIFY(rule1 != rule2);
    
    // 执行赋值
    rule2 = rule1;
    
    // 验证赋值后相等
    QVERIFY(rule1 == rule2);
    QCOMPARE(rule2.getRuleType(), RuleType::INCLUDES);
    QCOMPARE(rule2.getRuleField(), RuleField::TITLE);
    QCOMPARE(rule2.getRuleName(), QString("规则1"));
}

void TestLyricIgnoreRule::testEqualityOperator()
{
    LyricIgnoreRule rule1(RuleType::INCLUDES, RuleField::TITLE, "测试");
    LyricIgnoreRule rule2(RuleType::INCLUDES, RuleField::TITLE, "测试");
    LyricIgnoreRule rule3(RuleType::EXCLUDES, RuleField::TITLE, "测试");
    LyricIgnoreRule rule4(RuleType::INCLUDES, RuleField::ARTIST, "测试");
    LyricIgnoreRule rule5(RuleType::INCLUDES, RuleField::TITLE, "不同");
    
    // 相同规则应该相等
    QVERIFY(rule1 == rule2);
    
    // 不同规则类型应该不相等
    QVERIFY(!(rule1 == rule3));
    
    // 不同规则字段应该不相等
    QVERIFY(!(rule1 == rule4));
    
    // 不同规则名称应该不相等
    QVERIFY(!(rule1 == rule5));
}

void TestLyricIgnoreRule::testInequalityOperator()
{
    LyricIgnoreRule rule1(RuleType::INCLUDES, RuleField::TITLE, "测试");
    LyricIgnoreRule rule2(RuleType::INCLUDES, RuleField::TITLE, "测试");
    LyricIgnoreRule rule3(RuleType::EXCLUDES, RuleField::ARTIST, "不同");
    
    // 相同规则不应该不相等
    QVERIFY(!(rule1 != rule2));
    
    // 不同规则应该不相等
    QVERIFY(rule1 != rule3);
}

void TestLyricIgnoreRule::testGetters()
{
    LyricIgnoreRule rule(RuleType::EXCLUDES, RuleField::ALBUM, "测试专辑名");
    
    QCOMPARE(rule.getRuleType(), RuleType::EXCLUDES);
    QCOMPARE(rule.getRuleField(), RuleField::ALBUM);
    QCOMPARE(rule.getRuleName(), QString("测试专辑名"));
    
    // 测试字符串表示
    QCOMPARE(rule.getRuleTypeStr(), QString("排除"));
    QCOMPARE(rule.getRuleFieldStr(), QString("专辑"));
}

void TestLyricIgnoreRule::testStringConversion()
{
    // 测试规则字段转换
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::TITLE), QString("名称"));
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::ARTIST), QString("艺术家"));
    QCOMPARE(LyricIgnoreRule::lyricRulesToString(RuleField::ALBUM), QString("专辑"));
    
    // 测试规则类型转换
    LyricIgnoreRule includeRule(RuleType::INCLUDES, RuleField::TITLE, "测试");
    LyricIgnoreRule excludeRule(RuleType::EXCLUDES, RuleField::TITLE, "测试");
    
    QCOMPARE(includeRule.getRuleTypeStr(), QString("包含"));
    QCOMPARE(excludeRule.getRuleTypeStr(), QString("排除"));
}

void TestLyricIgnoreRule::testStringToEnum()
{
    // 测试字符串到规则字段的转换
    QCOMPARE(LyricIgnoreRule::stringToLyricRules("名称"), RuleField::TITLE);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules("艺术家"), RuleField::ARTIST);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules("专辑"), RuleField::ALBUM);
    
    // 测试无效字符串（应该返回默认值）
    QCOMPARE(LyricIgnoreRule::stringToLyricRules("无效"), RuleField::TITLE);
    QCOMPARE(LyricIgnoreRule::stringToLyricRules(""), RuleField::TITLE);
    
    // 测试字符串到规则类型的转换
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules("包含"), RuleType::INCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules("排除"), RuleType::EXCLUDES);
    
    // 测试无效字符串（应该返回默认值）
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules("无效"), RuleType::INCLUDES);
    QCOMPARE(LyricIgnoreRule::stringToIgnoreRules(""), RuleType::INCLUDES);
}

void TestLyricIgnoreRule::testSetRulesStr()
{
    LyricIgnoreRule rule(RuleType::INCLUDES, RuleField::TITLE, "测试规则");
    
    // setRulesStr应该根据枚举值设置字符串表示
    // 由于这是一个内部方法，我们通过验证getRuleTypeStr和getRuleFieldStr来测试
    QCOMPARE(rule.getRuleTypeStr(), QString("包含"));
    QCOMPARE(rule.getRuleFieldStr(), QString("名称"));
    
    // 创建不同类型的规则来测试所有组合
    LyricIgnoreRule excludeArtistRule(RuleType::EXCLUDES, RuleField::ARTIST, "测试");
    QCOMPARE(excludeArtistRule.getRuleTypeStr(), QString("排除"));
    QCOMPARE(excludeArtistRule.getRuleFieldStr(), QString("艺术家"));
    
    LyricIgnoreRule includeAlbumRule(RuleType::INCLUDES, RuleField::ALBUM, "测试");
    QCOMPARE(includeAlbumRule.getRuleTypeStr(), QString("包含"));
    QCOMPARE(includeAlbumRule.getRuleFieldStr(), QString("专辑"));
}

void TestLyricIgnoreRule::testEdgeCases()
{
    // 测试空规则名称
    LyricIgnoreRule emptyRule(RuleType::INCLUDES, RuleField::TITLE, "");
    QVERIFY(emptyRule.getRuleName().isEmpty());
    QCOMPARE(emptyRule.getRuleType(), RuleType::INCLUDES);
    QCOMPARE(emptyRule.getRuleField(), RuleField::TITLE);
    
    // 测试很长的规则名称
    QString longName = QString("很长的规则名称").repeated(100);
    LyricIgnoreRule longRule(RuleType::EXCLUDES, RuleField::ARTIST, longName);
    QCOMPARE(longRule.getRuleName(), longName);
    
    // 测试特殊字符
    QString specialChars = "!@#$%^&*()[]{}|;:'\",.<>?/~`";
    LyricIgnoreRule specialRule(RuleType::INCLUDES, RuleField::ALBUM, specialChars);
    QCOMPARE(specialRule.getRuleName(), specialChars);
}

void TestLyricIgnoreRule::testAllCombinations()
{
    // 测试所有规则类型和字段的组合
    QList<RuleType> ruleTypes = {RuleType::INCLUDES, RuleType::EXCLUDES};
    QList<RuleField> ruleFields = {RuleField::TITLE, RuleField::ARTIST, RuleField::ALBUM};
    
    for (RuleType ruleType : ruleTypes) {
        for (RuleField ruleField : ruleFields) {
            LyricIgnoreRule rule(ruleType, ruleField, "测试规则");
            
            QCOMPARE(rule.getRuleType(), ruleType);
            QCOMPARE(rule.getRuleField(), ruleField);
            QCOMPARE(rule.getRuleName(), QString("测试规则"));
            
            // 验证字符串表示不为空
            QVERIFY(!rule.getRuleTypeStr().isEmpty());
            QVERIFY(!rule.getRuleFieldStr().isEmpty());
        }
    }
}

void TestLyricIgnoreRule::testChineseRuleNames()
{
    // 测试各种中文规则名称
    QStringList chineseNames = {
        "测试歌曲",
        "周杰伦",
        "夜曲",
        "流行音乐",
        "中文歌词",
        "繁體中文",
        "简体中文"
    };
    
    for (const QString& name : chineseNames) {
        LyricIgnoreRule rule(RuleType::INCLUDES, RuleField::TITLE, name);
        QCOMPARE(rule.getRuleName(), name);
        
        // 验证中文名称的规则能正确比较
        LyricIgnoreRule sameRule(RuleType::INCLUDES, RuleField::TITLE, name);
        QVERIFY(rule == sameRule);
        
        LyricIgnoreRule differentRule(RuleType::EXCLUDES, RuleField::TITLE, name);
        QVERIFY(rule != differentRule);
    }
    
    // 测试混合中英文
    LyricIgnoreRule mixedRule(RuleType::INCLUDES, RuleField::ARTIST, "Jay周杰伦Chou");
    QCOMPARE(mixedRule.getRuleName(), QString("Jay周杰伦Chou"));
}

#include "TestLyricIgnoreRule.moc"
