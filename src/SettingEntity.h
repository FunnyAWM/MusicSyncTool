#ifndef SETTINGENTITY_H
#define SETTINGENTITY_H
#include <QString>

struct set {
	bool ignoreLyric;
	short sortBy;
	short orderBy;
	QString language;
	QString favoriteTag;
	bool dsEnabled;
};
#endif // SETTINGENTITY_H
