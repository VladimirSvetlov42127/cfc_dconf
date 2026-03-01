#pragma once

#include <qstring.h>

#include <data_model/storage_for_controller_params/dc_param.h>
#include <db/dc_db_wrapper.h>

class DcParamCfg_v2_Headline : public DcDbWrapper
{
public:
    DcParamCfg_v2_Headline(int32_t controllerid);
    DcParamCfg_v2_Headline(int32_t controllerid, const QString &name, DefParamDataType dataType, uint16_t address, attribType_t attributes, uint16_t profileCount, uint16_t subProfileCount, uint16_t dataLenInBytes);
	~DcParamCfg_v2_Headline();

	// Getters
	QString getName();
	DefParamDataType getDataType();
	uint16_t getAddress();
	attribType_t getAttributes();
	uint16_t getProfileCount();
	uint16_t getSubProfileCount();
	uint16_t getDataLenInBytes();

	// Setters
	void setName(QString newVal);
	void setDataType(DefParamDataType newVal);
	void setAddress(uint16_t newVal);
	void setAttributes(attribType_t newVal);
	void setProfileCount(uint16_t newVal);
	void setSubProfileCount(uint16_t newVal);
	void setDataLenInBytes(uint16_t newVal);

	virtual void insert();
	virtual void update();
	virtual void remove();

	bool operator==(const DcParamCfg_v2_Headline &h) const;

private:
	QString m_name;				// Главное имя 
	DefParamDataType m_dataType;		// Тип данных
	uint16_t m_address;			// Уникальный!
	attribType_t m_attributes;		// Чтение/запись, конфигурационные и пр.
	uint16_t m_profileCount;	// Количество профилей в данном адресе
	uint16_t m_subProfileCount;	// Кол-во параметров в каждом профиле
	uint16_t m_dataLenInBytes;	// Длина параметра в байтах
};

