#include "DcParamCfg_v2_Headline.h"
#include "qdebug.h"

#include <db/dc_db_manager.h>

DcParamCfg_v2_Headline::DcParamCfg_v2_Headline(int32_t controllerid)
    : DcDbWrapper(controllerid)
    , m_name("")
    , m_dataType(DefParamDataType::DEF_DATA_TYPE_UNDEF)
    , m_address(0)
    , m_attributes(0)
    , m_profileCount(0)
    , m_subProfileCount(0)
    , m_dataLenInBytes(0)
{
}

DcParamCfg_v2_Headline::DcParamCfg_v2_Headline(int32_t controllerid, const QString & name, DefParamDataType dataType, uint16_t address, attribType_t attributes,
	uint16_t profileCount, uint16_t subProfileCount, uint16_t dataLenInBytes)
    : DcDbWrapper(controllerid)
    , m_name(name)
    , m_dataType(dataType)
    , m_address(address)
    , m_attributes(attributes)
    , m_profileCount(profileCount)
    , m_subProfileCount(subProfileCount)
    , m_dataLenInBytes(dataLenInBytes)
{
}


DcParamCfg_v2_Headline::~DcParamCfg_v2_Headline()
{
}

QString DcParamCfg_v2_Headline::getName()
{
	return m_name;
}

DefParamDataType DcParamCfg_v2_Headline::getDataType()
{
	return m_dataType;
}

uint16_t DcParamCfg_v2_Headline::getAddress()
{
	return m_address;
}

attribType_t DcParamCfg_v2_Headline::getAttributes()
{
	return m_attributes;
}

uint16_t DcParamCfg_v2_Headline::getProfileCount()
{
	return m_profileCount;
}

uint16_t DcParamCfg_v2_Headline::getSubProfileCount()
{
	return m_subProfileCount;
}

uint16_t DcParamCfg_v2_Headline::getDataLenInBytes()
{
	return m_dataLenInBytes;
}

void DcParamCfg_v2_Headline::setName(QString newVal)
{
	m_name = newVal;
}

void DcParamCfg_v2_Headline::setDataType(DefParamDataType newVal)
{
	m_dataType = newVal;
}

void DcParamCfg_v2_Headline::setAddress(uint16_t newVal)
{
	m_address = newVal;
}

void DcParamCfg_v2_Headline::setAttributes(attribType_t newVal)
{
	m_attributes = newVal;
}

void DcParamCfg_v2_Headline::setProfileCount(uint16_t newVal)
{
	m_profileCount = newVal;
}

void DcParamCfg_v2_Headline::setSubProfileCount(uint16_t newVal)
{
	m_subProfileCount = newVal;
}

void DcParamCfg_v2_Headline::setDataLenInBytes(uint16_t newVal)
{
	m_dataLenInBytes = newVal;
}

void DcParamCfg_v2_Headline::insert()
{
	QString attr = getAttributeStringFromInt(getAttributes());
	QString temp = "INSERT INTO cfg_parameters_headline(name, type, address, attributes, profiles_count, subProfiles_count, subProfile_size_in_bytes ) VALUES('%1', %2, %3, '%4', %5, %6, %7);";
	QString insertStr = temp.arg(getName()).arg(getDataType()).arg(getAddress()).arg(attr).arg(getProfileCount()).arg(getSubProfileCount()).arg(getDataLenInBytes());
	gDbManager.execute(uid(), insertStr);
}

void DcParamCfg_v2_Headline::update()
{
	QString attr = getAttributeStringFromInt(getAttributes());
	QString temp = "UPDATE cfg_parameters_headline SET name = '%1', type = %2, attributes = '%3', profiles_count = %4, subProfiles_count = %5, subProfile_size_in_bytes = %6 WHERE address = %7;";
	QString updateStr = temp.arg(getName()).arg(getDataType()).arg(attr).arg(getProfileCount()).arg(getSubProfileCount()).arg(getDataLenInBytes()).arg(getAddress());
	gDbManager.execute(uid(), updateStr);
}

void DcParamCfg_v2_Headline::remove()
{
	QString temp = "DELETE FROM cfg_parameters_headline WHERE address = %1;";
	QString removeStr = temp.arg(getAddress());
	gDbManager.execute(uid(), removeStr);
}

bool DcParamCfg_v2_Headline::operator==(const DcParamCfg_v2_Headline & h) const
{
    return m_address == h.m_address
            && m_dataType == h.m_dataType
            && m_profileCount == h.m_profileCount
            && m_subProfileCount == h.m_subProfileCount
            && m_dataLenInBytes == h.m_dataLenInBytes
            && m_attributes == h.m_attributes
            && m_name == h.m_name;
}
