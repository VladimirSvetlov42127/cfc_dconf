/*
iconvlite.h
Iconv Lite
Simple cpp functions to convert strings from cp1251 to utf8 and ftom utf8 to cp1251
*/

#ifndef ICONVLITE_H
#define ICONVLITE_H

#include <dpc/dpc_global.h>
#include <qstring.h>

using namespace std;

class DPC_EXPORT conv
{
public:
	conv();
	//conv(QByteArray data, quint8 codec);
	~conv();

	QString toUtf8(QString s);
	QString toUtf8(QByteArray s);
	QString toW1251(QString s);
	QString toW1251(QByteArray s);

	//do not use
	bool isUtf8(QByteArray s);

//private:
//	QByteArray m_rawData;
//	quint8 m_codec;
};

#endif
