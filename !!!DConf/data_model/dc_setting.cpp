#include "dc_setting.h"
#include <db/dc_db_manager.h>


DcSetting::DcSetting(int32_t controllerid, const QString &name, const QString &value)
    : DcDbWrapper(controllerid)
    , m_name(name)
    , m_value(value)
{
	
}

DcSetting::~DcSetting()
{
}

QString DcSetting::name()
{
	return m_name;
}

QString DcSetting::value()
{
	return m_value;
}

bool DcSetting::updateValue(const QString & value)
{
	if (value.isEmpty())
		return false;
	m_value = value;
	update();
	return true;
}

////////////////////////////////////
// work with table core_properties 
////////////////////////////////////
void DcSetting::update()
{
	QString updateStrTmp = "UPDATE settings SET setting_value = '%1' WHERE setting_name = '%2';";
	QString updateStr = updateStrTmp.arg(m_value, m_name);
	gDbManager.execute(uid(), updateStr);
}

void DcSetting::insert()
{
	QString insertStrTmp = "INSERT INTO settings(setting_name, setting_value) VALUES('%1', '%2');";
	QString insertStr = insertStrTmp.arg(m_name, m_value);
	gDbManager.execute(uid(), insertStr);
}

void DcSetting::remove()
{
	QString removeStrTmp = "DELETE FROM settings WHERE setting_name = '%1';";
	QString removeStr = removeStrTmp.arg(m_name);
	gDbManager.execute(uid(), removeStr);
}
