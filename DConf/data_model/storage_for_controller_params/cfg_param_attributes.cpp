#include "cfg_param_attributes.h"
#include <qlist.h>
#include <qstring.h>


QString getAttributeStringFromInt(attribType_t flags) {
	QString atrrString = "";
	for (int i = 0; i < attributesFlags.size(); i++) {		// Проходим по всем флагам
		if (flags & attributesFlags[i].first) {				// Если совпадает флаг
			atrrString += attributesFlags[i].second + ",";	// Ставим флаг и разделитель
		}
	}

	/// Это уже должно было быть урезано при вычитывании из БД имен флагов! Файл db_load_cfg_params.cpp
	//if (flags & param_flags_e::READ && !isAttributeInInt(flags, ATTRIBUTE_READ)) {
	//	atrrString += ATTRIBUTE_READ + ",";
	//}
	//if (flags & param_flags_e::WRITE && !isAttributeInInt(flags, ATTRIBUTE_WRITE)) {
	//	atrrString += ATTRIBUTE_WRITE + ",";
	//}

	atrrString.resize(atrrString.size() - 1);				// Убираем лишние символы (',')
	return atrrString;
}

attribType_t getAttributeIntFromString(QString flagstr) {
	attribType_t flags = 0;

	if (flagstr.contains("rw")) {	// Поддержка старого формата
		flags = attributesFlags[3].first + attributesFlags[4].first;
	}

	QList<QString> atrList = flagstr.split(',');
	for (QString oneAtr : atrList) {
		for (QPair<attribType_t, QString> flagsPair : attributesFlags) {		// Проходим по всем флагам
			if (oneAtr == flagsPair.second) {				// Если совпадает флаг
				flags |= flagsPair.first;	// Ставим флаг и разделитель
			}
		}
	}
	return flags;
}

bool isAttributeInInt(attribType_t allFlags, QString chosenFlagName) {
	if (chosenFlagName.isEmpty()) return false;

	attribType_t chosenFlagInt = 0;
	for (QPair<attribType_t, QString> flagsPair : attributesFlags) {
		if (flagsPair.second == chosenFlagName) {
			chosenFlagInt = flagsPair.first;
			break;
		}
	}

	if (allFlags & chosenFlagInt) return true;
	return false;
}

bool isAttributeInString(QString flagstr, QString chosenFlagName) {
	QList<QString> atrList = flagstr.split(',');
	for (QString oneAtr : atrList) {
		if (oneAtr == chosenFlagName) {
			return true;
		}
	}
	return false;
}

attribType_t getMaskForAttribute(QString flagstr)
{
	for (int i = 0; i < attributesFlags.size(); i++) {
		if (flagstr == attributesFlags[i].second) {
			return attributesFlags[i].first;
		}
	}
	return 0;
}
