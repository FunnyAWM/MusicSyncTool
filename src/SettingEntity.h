#ifndef SETTINGENTITY_H
#define SETTINGENTITY_H
#include <QMap>
#include <QString>
#include "MusicProperties.h"
using PROPERTIES::lyricRules;
using PROPERTIES::ignoreLyricRules;

struct set {
	bool ignoreLyric;
	short sortBy;
	short orderBy;
	QString language;
	QString favoriteTag;
	QList<LyricIgnoreRuleSingleton> rules;
};
#endif // SETTINGENTITY_H
