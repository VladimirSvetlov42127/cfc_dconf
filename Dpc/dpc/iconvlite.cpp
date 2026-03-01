#include "iconvlite.h"

// #include <iostream>

// #include <codecvt>
// #include <locale>


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(QT_CORE5COMPAT_LIB)
#include "qtextcodec.h"
#else
#define USE_STD
#ifndef USE_STD
#include "qstringconverter.h"
#endif //USE_STD
#endif //QT_CORE5COMPAT_LIB


#ifdef USE_STD
using namespace std;

typedef struct ConvLetter {
	int	 win1251;
	int	 unicode;
} Letter;

static Letter g_letters[] = {
	/* 0x80 */
	{0x80, 0x0402}, // ???
	{0x81, 0x0403}, // ???
	{0x82, 0x201A}, // SINGLE LOW-9 QUOTATION MARK
	{0x83, 0x0453}, // CYRILLIC SMALL LETTER GJE
	{0x84, 0x201E}, // DOUBLE LOW-9 QUOTATION MARK
	{0x85, 0x2026}, // HORIZONTAL ELLIPSIS
	{0x86, 0x2020}, // DAGGER
	{0x87, 0x2021}, // DOUBLE DAGGER
	{0x88, 0x20AC}, // EURO SIGN
	{0x89, 0x2030}, // PER MILLE SIGN
	{0x8A, 0x0409}, // CYRILLIC CAPITAL LETTER LJE
	{0x8B, 0x2039}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	{0x8C, 0x040A}, // CYRILLIC CAPITAL LETTER NJE
	{0x8D, 0x040C}, // CYRILLIC CAPITAL LETTER KJE
	{0x8E, 0x040B}, // CYRILLIC CAPITAL LETTER TSHE
	{0x8F, 0x040F}, // CYRILLIC CAPITAL LETTER DZHE
	/* 0x90 */
	{0x90, 0x0452}, // CYRILLIC SMALL LETTER DJE
	{0x91, 0x2018}, // LEFT SINGLE QUOTATION MARK
	{0x92, 0x2019}, // RIGHT SINGLE QUOTATION MARK
	{0x93, 0x201C}, // LEFT DOUBLE QUOTATION MARK
	{0x94, 0x201D}, // RIGHT DOUBLE QUOTATION MARK
	{0x95, 0x2022}, // BULLET
	{0x96, 0x2013}, // EN DASH
	{0x97, 0x2014}, // EM DASH
	{0x98, 0x007F}, // ??????????
	{0x99, 0x2122}, // TRADE MARK SIGN
	{0x9A, 0x0459}, // CYRILLIC SMALL LETTER LJE
	{0x9B, 0x203A}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	{0x9C, 0x045A}, // CYRILLIC SMALL LETTER NJE
	{0x9D, 0x045C}, // CYRILLIC SMALL LETTER KJE
	{0x9E, 0x045B}, // CYRILLIC SMALL LETTER TSHE
	{0x9F, 0x045F}, // CYRILLIC SMALL LETTER DZHE
	/* 0xa0 */
	{0xA0, 0x00A0}, // NO-BREAK SPACE
	{0xA1, 0x040E}, // CYRILLIC CAPITAL LETTER SHORT U
	{0xA2, 0x045E}, // CYRILLIC SMALL LETTER SHORT U
	{0xA3, 0x0408}, // CYRILLIC CAPITAL LETTER JE
	{0xA4, 0x00A4}, // CURRENCY SIGN
	{0xA5, 0x0490}, // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
	{0xA6, 0x00A6}, // BROKEN BAR
	{0xA7, 0x00A7}, // SECTION SIGN
	{0xA8, 0x0401}, // CYRILLIC CAPITAL LETTER IO
	{0xA9, 0x00A9}, // COPYRIGHT SIGN
	{0xAA, 0x0404}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
	{0xAB, 0x00AB}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	{0xAC, 0x00AC}, // NOT SIGN
	{0xAD, 0x00AD}, // SOFT HYPHEN
	{0xAE, 0x00AE}, // REGISTERED SIGN
	{0xAF, 0x0407}, // CYRILLIC CAPITAL LETTER YI
	/* 0xb0 */
	{0xB0, 0x00B0}, // DEGREE SIGN
	{0xB1, 0x00B1}, // PLUS-MINUS SIGN
	{0xB2, 0x0406}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
	{0xB3, 0x0456}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
	{0xB4, 0x0491}, // CYRILLIC SMALL LETTER GHE WITH UPTURN
	{0xB5, 0x00B5}, // MICRO SIGN
	{0xB6, 0x00B6}, // PILCROW SIGN
	{0xB7, 0x00B7}, // MIDDLE DOT
	{0xB8, 0x0451}, // CYRILLIC SMALL LETTER IO
	{0xB9, 0x2116}, // NUMERO SIGN
	{0xBA, 0x0454}, // CYRILLIC SMALL LETTER UKRAINIAN IE
	{0xBB, 0x00BB}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	{0xBC, 0x0458}, // CYRILLIC SMALL LETTER JE
	{0xBD, 0x0405}, // CYRILLIC CAPITAL LETTER DZE
	{0xBE, 0x0455}, // CYRILLIC SMALL LETTER DZE
	{0xBF, 0x0457}, // CYRILLIC SMALL LETTER YI
	/* 0xc0 */
	{0xC0, 0x0410},
	{0xC1, 0x0411},
	{0xC2, 0x0412},
	{0xC3, 0x0413},
	{0xC4, 0x0414},
	{0xC5, 0x0415},
	{0xC6, 0x0416},
	{0xC7, 0x0417},
	{0xC8, 0x0418},
	{0xC9, 0x0419},
	{0xCA, 0x041a},
	{0xCB, 0x041b},
	{0xCC, 0x041c},
	{0xCD, 0x041d},
	{0xCE, 0x041e},
	{0xCF, 0x041f},
	/* 0xd0 */
	{0xD0, 0x0420},
	{0xD1, 0x0421},
	{0xD2, 0x0422},
	{0xD3, 0x0423},
	{0xD4, 0x0424},
	{0xD5, 0x0425},
	{0xD6, 0x0426},
	{0xD7, 0x0427},
	{0xD8, 0x0428},
	{0xD9, 0x0429},
	{0xDA, 0x042a},
	{0xDB, 0x042b},
	{0xDC, 0x042c},
	{0xDD, 0x042d},
	{0xDE, 0x042e},
	{0xDF, 0x042f},
	/* 0xe0 */
	{ 0xE0, 0x0430 },
	{ 0xE1, 0x0431 },
	{ 0xE2, 0x0432 },
	{ 0xE3, 0x0433 },
	{ 0xE4, 0x0434 },
	{ 0xE5, 0x0435 },
	{ 0xE6, 0x0436 },
	{ 0xE7, 0x0437 },
	{ 0xE8, 0x0438 },
	{ 0xE9, 0x0439 },
	{ 0xEA, 0x043a },
	{ 0xEB, 0x043b },
	{ 0xEC, 0x043c },
	{ 0xED, 0x043d },
	{ 0xEE, 0x043e },
	{ 0xEF, 0x043f },
	/* 0xf0 */
	{ 0xF0, 0x0440 },
	{ 0xF1, 0x0441 },
	{ 0xF2, 0x0442 },
	{ 0xF3, 0x0443 },
	{ 0xF4, 0x0444 },
	{ 0xF5, 0x0445 },
	{ 0xF6, 0x0446 },
	{ 0xF7, 0x0447 },
	{ 0xF8, 0x0448 },
	{ 0xF9, 0x0449 },
	{ 0xFA, 0x044a },
	{ 0xFB, 0x044b },
	{ 0xFC, 0x044c },
	{ 0xFD, 0x044d },
	{ 0xFE, 0x044e },
	{ 0xFF, 0x044f },

};
#endif //USE_STD

//#ifdef QT_DEBUG
//#include <qfile.h>
//#endif //QT_DEBUG


conv::conv() //: conv(QByteArray(), 0)
{

}
//conv::conv(QByteArray data, quint8 codec) : m_rawData(data), m_codec(codec)
//{
//}

conv::~conv()
{
	//m_rawData.clear();
}

QString conv::toUtf8(QString s)
{
	QByteArray encodedString;// = s.toUtf8();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(QT_CORE5COMPAT_LIB)
	QTextCodec* codec_1251 = QTextCodec::codecForName("Windows-1251");
	encodedString = codec_1251->fromUnicode(s);
#else
#ifndef USE_STD
	QStringEncoder* codec_1251 = new QStringEncoder(QStringDecoder::System); //new QStringEncoder("Windows-1251");
	encodedString = codec_1251->encode(s);
#else
	encodedString = s.toUtf8();
#endif //USE_STD
#endif //QT_CORE5COMPAT_LIB
	return toUtf8(encodedString);
}

QString conv::toUtf8(QByteArray s)
{
	if (isUtf8(s))
		return  s;
	QString convertedString;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(QT_CORE5COMPAT_LIB)
	QTextCodec* codec_1251 = QTextCodec::codecForName("Windows-1251");
	convertedString = codec_1251->toUnicode(s);
#else
#ifndef USE_STD
	QStringDecoder* codec_1251 = new QStringDecoder(QStringDecoder::System); //new QStringDecoder("Windows-1251");
	convertedString = codec_1251->decode(s);
#else
	static const int table[128] = //что за таблица???
	{
		0x82D0,0x83D0,0x9A80E2,0x93D1,0x9E80E2,0xA680E2,0xA080E2,0xA180E2,
		0xAC82E2,0xB080E2,0x89D0,0xB980E2,0x8AD0,0x8CD0,0x8BD0,0x8FD0,
		0x92D1,0x9880E2,0x9980E2,0x9C80E2,0x9D80E2,0xA280E2,0x9380E2,0x9480E2,
		0,0xA284E2,0x99D1,0xBA80E2,0x9AD1,0x9CD1,0x9BD1,0x9FD1,
		0xA0C2,0x8ED0,0x9ED1,0x88D0,0xA4C2,0x90D2,0xA6C2,0xA7C2,
		0x81D0,0xA9C2,0x84D0,0xABC2,0xACC2,0xADC2,0xAEC2,0x87D0,
		0xB0C2,0xB1C2,0x86D0,0x96D1,0x91D2,0xB5C2,0xB6C2,0xB7C2,
		0x91D1,0x9684E2,0x94D1,0xBBC2,0x98D1,0x85D0,0x95D1,0x97D1,
		0x90D0,0x91D0,0x92D0,0x93D0,0x94D0,0x95D0,0x96D0,0x97D0,
		0x98D0,0x99D0,0x9AD0,0x9BD0,0x9CD0,0x9DD0,0x9ED0,0x9FD0,
		0xA0D0,0xA1D0,0xA2D0,0xA3D0,0xA4D0,0xA5D0,0xA6D0,0xA7D0,
		0xA8D0,0xA9D0,0xAAD0,0xABD0,0xACD0,0xADD0,0xAED0,0xAFD0,
		0xB0D0,0xB1D0,0xB2D0,0xB3D0,0xB4D0,0xB5D0,0xB6D0,0xB7D0,
		0xB8D0,0xB9D0,0xBAD0,0xBBD0,0xBCD0,0xBDD0,0xBED0,0xBFD0,
		0x80D1,0x81D1,0x82D1,0x83D1,0x84D1,0x85D1,0x86D1,0x87D1,
		0x88D1,0x89D1,0x8AD1,0x8BD1,0x8CD1,0x8DD1,0x8ED1,0x8FD1
	 };
	char c;
	int i;
	int len = s.size();
	string ns;
	for(i=0; i<len; i++) {
		c=s[i];
		char buf1[4] = { 0, 0, 0, 0 };
		char in1[2] = {0, 0};
		*in1 = c;
		const char * in = in1;
		char * buf = buf1;
		//cp2utf1(buf, in);

		while (*in)
			if (*in & 0x80) {
				int v = table[(int)(0x7f & *in++)];
				if (!v)
					 continue;
				*buf++ = (char)v;
				*buf++ = (char)(v >> 8);
				if (v >>= 16)
					 *buf++ = (char)v;
			}
			else
				*buf++ = *in++;
		*buf = 0;


		ns+=string(buf1);
	}
	convertedString = ns.c_str();
#endif //USE_STD
#endif //QT_CORE5COMPAT_LIB
	return convertedString.toUtf8();
}

QString conv::toW1251(QString s)
{
	QByteArray encodedString;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(QT_CORE5COMPAT_LIB)
	QTextCodec* codec_utf8 = QTextCodec::codecForName("UTF8");
	encodedString = codec_utf8->fromUnicode(s);
#else
#ifndef USE_STD
	QStringEncoder* codec_utf8 = new QStringEncoder("UTF8");
	encodedString = codec_utf8->encode(s);
#else
	encodedString = s.toUtf8();
#endif //USE_STD
#endif //QT_CORE5COMPAT_LIB
#ifdef QT_DEBUG
	//QFile f(QString("./logs/tmp.enc"));// .arg(connector->DbName().replace("/", ".").replace(":", ".").replace("\\", ".")));
	//f.open(QIODevice::Append);
	//f.write(encodedString);
	////f.write(string.toUtf8());
	//f.close();
#endif //QT_DEBUG
	return toW1251(encodedString);
}

QString conv::toW1251(QByteArray s)
{
	if (!isUtf8(s))
		return  s;
	QString convertedString;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(QT_CORE5COMPAT_LIB)
	QTextCodec* codec_utf8 = QTextCodec::codecForName("UTF8");
	convertedString = codec_utf8->toUnicode(s);
#else
#ifndef USE_STD
	QStringDecoder* codec_utf8 = new QStringDecoder("UTF8");
	encodedString = codec_utf8->decode(s);
#else

	size_t len = s.size();
	 const char * utf8 = s.data();
	 char * windows1251 = new char[len];
	// convert_utf8_to_windows1251(buff, output, len);

	int i = 0;
	int j = 0;
	for(; i < (int)len && utf8[i] != 0; ++i) {
		char prefix = utf8[i];
		char suffix = utf8[i+1];
		if ((prefix & 0x80) == 0) {
			windows1251[j] = (char)prefix;
			++j;
		} else if ((~prefix) & 0x20) {
			int first5bit = prefix & 0x1F;
			first5bit <<= 6;
			int sec6bit = suffix & 0x3F;
			int unicode_char = first5bit + sec6bit;

			if ( unicode_char >= 0x410 && unicode_char <= 0x44F ) {
				windows1251[j] = (char)(unicode_char - 0x350);
			} else if (unicode_char >= 0x80 && unicode_char <= 0xFF) {
				windows1251[j] = (char)(unicode_char);
			} else if (unicode_char >= 0x402 && unicode_char <= 0x403) {
				windows1251[j] = (char)(unicode_char - 0x382);
			} else {
				int count = sizeof(g_letters) / sizeof(Letter);
				bool isFind = false;
				for (int k = 0; k < count; ++k) {
					if (unicode_char == g_letters[k].unicode) {
						windows1251[j] = (char)g_letters[k].win1251;
						isFind = true;
						//goto NEXT_LETTER;
					}
				}
				// can't convert this char
				if (!isFind)
					windows1251[j] = ' ';
			}
//NEXT_LETTER:
			++i;
			++j;
		} else {
				// can't convert this chars
				return "";
		}
	}
	windows1251[j] = 0;
	//return 1;


	 //string ns(output);
	 //return ns;

	 convertedString = windows1251;
#endif //USE_STD
#endif //QT_CORE5COMPAT_LIB
	return convertedString;
}

bool conv::isUtf8(QByteArray s)
{
	const char* string = s.data();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(QT_CORE5COMPAT_LIB)
	QTextCodec::ConverterState state;
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	const QString text = codec->toUnicode(string, s.size(), &state);
	if (state.invalidChars > 0) {
		return false;
	}
#else
#ifndef USE_STD
	return QStringConverter::encodingForData(string) == QStringConverter::Utf8;
#else
	if (!string) { return true; }
	const unsigned char* bytes = (const unsigned char*)string;
	unsigned int cp;
	int num;
	while (*bytes != 0x00) {
		if ((*bytes & 0x80) == 0x00) {
			// U+0000 to U+007F 
			cp = (*bytes & 0x7F);
			num = 1;
		}
		else if ((*bytes & 0xE0) == 0xC0) {
			// U+0080 to U+07FF 
			cp = (*bytes & 0x1F);
			num = 2;
		}
		else if ((*bytes & 0xF0) == 0xE0) {
			// U+0800 to U+FFFF 
			cp = (*bytes & 0x0F);
			num = 3;
		}
		else if ((*bytes & 0xF8) == 0xF0) {
			// U+10000 to U+10FFFF 
			cp = (*bytes & 0x07);
			num = 4;
		}
		else { return false; }
		bytes += 1;
		for (int i = 1; i < num; ++i) {
			if ((*bytes & 0xC0) != 0x80) { return false; }
			cp = (cp << 6) | (*bytes & 0x3F);
			bytes += 1;
		}
		if ((cp > 0x10FFFF) ||
			((cp <= 0x007F) && (num != 1)) ||
			((cp >= 0xD800) && (cp <= 0xDFFF)) ||
			((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
			((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
			((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4))) {
			return false;
		}
	}
#endif //USE_STD
#endif //QT_CORE5COMPAT_LIB
	return true;
}
