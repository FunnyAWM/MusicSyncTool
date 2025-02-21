#ifndef SETTINGENTITY_H
#define SETTINGENTITY_H
#include <QString>
#include "MusicProperties.h"
using PROPERTIES::RuleField;
using PROPERTIES::RuleType;

struct set {
	bool ignoreLyric;
	short sortBy;
	short orderBy;
	QString language;
	QString favoriteTag;
	QList<LyricIgnoreRule> rules;
};
#endif // SETTINGENTITY_H
