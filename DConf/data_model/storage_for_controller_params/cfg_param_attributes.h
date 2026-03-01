#pragma once
#include <qvector.h>
#include <qpair.h>
#include <qstring.h>

typedef uint16_t attribType_t;

static const QString ATTRIBUTE_READ = "r";	// Определение для поиска вхождения в список 
static const QString ATTRIBUTE_WRITE = "w";
static const QString ATTRIBUTE_CFG = "cfg";

//const enum param_flags_e {	// Старые флаги, которые были упразднены. Исправлено в файле dc_load_cfg_params.cpp
//	READ = 1,
//	WRITE = 2
//};

static const QVector <QPair<attribType_t, QString>> attributesFlags{				// Атрибуты для флагов параметров. Первое поле - битовая маска флага, второе - буквенное обозначение для записи в строку
	{1 << 3, "wnd"},	/*!< Write not defence (not guarantee) */
	{1 << 4, "lcl"},	/* Local Parametr ( no remote service*/
	{1 << 5, "rst"},	/* Reset device after change */
	{1 << 6, ATTRIBUTE_READ},		/* Read enable */
	{1 << 7, ATTRIBUTE_WRITE},		/* Write enable */
	{1 << 8, "pn"},		/* Network password write protect */
	{1 << 9, "ps_man"},	/* Minipult password write protect */
	{1 << 10, "2d"},	/* Two dimention array parametr */
	{1 << 11, "ps"},	/* System Password Write protect*/
	{1 << 12, ATTRIBUTE_CFG},	/* Configuration parametr	*/
	{1 << 13, "e"},		/* Support extended request mode */
	{1 << 14, "nw"},	/* Parameter Name write enable */
	{1 << 15, "n"},		/* Parameter have Name*/	
};


QString getAttributeStringFromInt(attribType_t flags);
attribType_t getAttributeIntFromString(QString flagstr);
bool isAttributeInInt(attribType_t allFlags, QString chosenFlagName);
bool isAttributeInString(QString flagstr, QString chosenFlagName);

attribType_t getMaskForAttribute(QString flagstr);