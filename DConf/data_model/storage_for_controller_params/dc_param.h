#pragma once
#include <stdint.h>
#include <qstring.h>
#include <data_model/storage_for_controller_params/cfg_param_attributes.h>	// Для определения типа данных флагов атрибутов

#define PROFILE_SIZE 256

#define DATA_TYPE_UNDEF "Undef"
#define DATA_TYPE_BYTE "Byte"
#define DATA_TYPE_CHAR "Char"
#define DATA_TYPE_BYTEBOOL "Byte bool"
#define DATA_TYPE_SHORT "Short"
#define DATA_TYPE_WORD "Word"
#define DATA_TYPE_BOOL "Bool"
#define DATA_TYPE_UINT "UInt"
#define DATA_TYPE_INTEGER "Integer"
#define DATA_TYPE_DWORD "DWord"
#define DATA_TYPE_LONG "Long"
#define DATA_TYPE_FLOAT "Float"
#define DATA_TYPE_DATETIME "Date time"
#define DATA_TYPE_STRING "String"
#define DATA_TYPE_OUI "MAC adr"
#define DATA_TYPE_IP4ADR "IP4 adr"
#define DATA_TYPE_UTC "UTC time"
#define DATA_TYPE_T_SBYTE "T_SByte"
#define DATA_TYPE_T_8BIT "T_8BIT"
#define DATA_TYPE_T_16BIT "T_16BIT"
#define DATA_TYPE_T_32BIT "T_32Bit"

typedef enum DefParamDataType {
	DEF_DATA_TYPE_UNDEF = 0,
	DEF_DATA_TYPE_BYTE = 0x01,
	DEF_DATA_TYPE_CHAR = 0x02,
	DEF_DATA_TYPE_BYTEBOOL = 0x03,
	DEF_DATA_TYPE_SHORT = 0x04,
	DEF_DATA_TYPE_WORD = 0x05,
	DEF_DATA_TYPE_BOOL = 0x06,
	DEF_DATA_TYPE_UNIT = 0x07,
	DEF_DATA_TYPE_INTEGER = 0x08,
	DEF_DATA_TYPE_DWORD = 0x09,
	DEF_DATA_TYPE_LONG = 0x0A,
	DEF_DATA_TYPE_FLOAT = 0x0B,
	DEF_DATA_TYPE_DATETIME = 0x10,
	DEF_DATA_TYPE_STRING = 0x11,
	DEF_DATA_TYPE_OUI = 0x17, //6 байт  - MAC адрес
	DEF_DATA_TYPE_IP4ADR = 0x18, //IP4
	DEF_DATA_TYPE_UTC = 0x19,
	DEF_DATA_TYPE_T_SBYTE = 0x1A,
	DEF_DATA_TYPE_T_8BIT = 0x81,
	DEF_DATA_TYPE_T_16BIT = 0x85,
	DEF_DATA_TYPE_T_32BIT = 0x89,
}DefParamDataType;

static const QString convertDataTypeFromIntToString(DefParamDataType type) {
	QString result;
	switch (type)
	{
	case DEF_DATA_TYPE_UNDEF:
		result = DATA_TYPE_UNDEF;
		break;
	case DEF_DATA_TYPE_BYTE:
		result = DATA_TYPE_BYTE;
		break;
	case DEF_DATA_TYPE_CHAR:
		result = DATA_TYPE_CHAR;
		break;
	case DEF_DATA_TYPE_BYTEBOOL:
		result = DATA_TYPE_BYTEBOOL;
		break;
	case DEF_DATA_TYPE_SHORT:
		result = DATA_TYPE_SHORT;
		break;
	case DEF_DATA_TYPE_WORD:
		result = DATA_TYPE_WORD;
		break;
	case DEF_DATA_TYPE_BOOL:
		result = DATA_TYPE_BOOL;
		break;
	case DEF_DATA_TYPE_UNIT:
		result = DATA_TYPE_UINT;
		break;
	case DEF_DATA_TYPE_INTEGER:
		result = DATA_TYPE_INTEGER;
		break;
	case DEF_DATA_TYPE_DWORD:
		result = DATA_TYPE_DWORD;
		break;
	case DEF_DATA_TYPE_LONG:
		result = DATA_TYPE_LONG;
		break;
	case DEF_DATA_TYPE_FLOAT:
		result = DATA_TYPE_FLOAT;
		break;
	case DEF_DATA_TYPE_DATETIME:
		result = DATA_TYPE_DATETIME;
		break;
	case DEF_DATA_TYPE_STRING:
		result = DATA_TYPE_STRING;
		break;
	case DEF_DATA_TYPE_OUI:
		result = DATA_TYPE_OUI;
		break;
	case DEF_DATA_TYPE_IP4ADR:
		result = DATA_TYPE_IP4ADR;
		break;
	case DEF_DATA_TYPE_UTC:
		result = DATA_TYPE_UTC;
		break;
	case DEF_DATA_TYPE_T_SBYTE:
		result = DATA_TYPE_T_SBYTE;
		break;
	case DEF_DATA_TYPE_T_8BIT:
		result = DATA_TYPE_T_8BIT;
		break;
	case DEF_DATA_TYPE_T_16BIT:
		result = DATA_TYPE_T_16BIT;
		break;
	case DEF_DATA_TYPE_T_32BIT:
		result = DATA_TYPE_T_32BIT;
		break;
	default:
		break;
	}

	return result;
}

static  DefParamDataType convertDataTypeFromStringToInt(QString type) {
	DefParamDataType result;

	if (type.isEmpty())
		return DEF_DATA_TYPE_UNDEF;

	if (type == DATA_TYPE_UNDEF)
		result = DEF_DATA_TYPE_UNDEF;
	else if (type == DATA_TYPE_BYTE)
		result = DEF_DATA_TYPE_BYTE;
	else if (type == DATA_TYPE_CHAR)
		result = DEF_DATA_TYPE_CHAR;
	else if (type == DATA_TYPE_BYTEBOOL)
		result = DEF_DATA_TYPE_BYTEBOOL;
	else if (type == DATA_TYPE_SHORT)
		result = DEF_DATA_TYPE_SHORT;
	else if (type == DATA_TYPE_WORD)
		result = DEF_DATA_TYPE_WORD;
	else if (type == DATA_TYPE_BOOL)
		result = DEF_DATA_TYPE_BOOL;
	else if (type == DATA_TYPE_UINT)
		result = DEF_DATA_TYPE_UNIT;
	else if (type == DATA_TYPE_INTEGER)
		result = DEF_DATA_TYPE_INTEGER;
	else if (type == DATA_TYPE_DWORD)
		result = DEF_DATA_TYPE_DWORD;
	else if (type == DATA_TYPE_LONG)
		result = DEF_DATA_TYPE_LONG;
	else if (type == DATA_TYPE_FLOAT)
		result = DEF_DATA_TYPE_FLOAT;
	else if (type == DATA_TYPE_DATETIME)
		result = DEF_DATA_TYPE_DATETIME;
	else if (type == DATA_TYPE_STRING)
		result = DEF_DATA_TYPE_STRING;
	else if (type == DATA_TYPE_OUI)
		result = DEF_DATA_TYPE_OUI;
	else if (type == DATA_TYPE_IP4ADR)
		result = DEF_DATA_TYPE_IP4ADR;
	else if (type == DATA_TYPE_UTC)
		result = DEF_DATA_TYPE_UTC;
	else if (type == DATA_TYPE_T_SBYTE)
		result = DEF_DATA_TYPE_T_SBYTE;
	else if (type == DATA_TYPE_T_8BIT)
		result = DEF_DATA_TYPE_T_8BIT;
	else if (type == DATA_TYPE_T_16BIT)
		result = DEF_DATA_TYPE_T_16BIT;
	else if (type == DATA_TYPE_T_32BIT)
		result = DEF_DATA_TYPE_T_32BIT;
	else result = DEF_DATA_TYPE_UNDEF;

	return result;
}

class DcParam {
public:
	DcParam(int32_t addr, int32_t param, QString name, DefParamDataType type, attribType_t flags, QString value);

	virtual int32_t addr();
	virtual int32_t param();
	virtual QString name();
	virtual DefParamDataType type();
	virtual attribType_t flags();
	virtual QString value();
	virtual bool was_changed();						// Если параметр был обновлен, ставим true. После процесса чтения все должны становитсья false
	virtual void reset_param_changes();				// Сбрасывает флаг, что параметр был изменен

	virtual bool updateValue(QString newval);
	virtual bool updateName(QString newname);
	virtual void updatedb() = 0;
private:
	int32_t m_addr;
	int32_t m_param;
	QString m_name;
	DefParamDataType m_type;
	attribType_t m_flags; // Флаги определены в файле cfg_param_attributes.h
	QString m_value;
	bool was_changed_bool;					// Меняется при Update
};
