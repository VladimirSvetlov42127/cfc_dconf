#include "dc_param_cfg.h"
#include <db/dc_db_manager.h>

DcParamCfg::DcParamCfg(int32_t controllerid, int32_t addr, int32_t param, const QString &name, DefParamDataType type, attribType_t flags, const QString &value, const QString &properties)
    : DcParam(addr, param, name, type, flags, value)
    , DcDbWrapper(controllerid)
{
	m_properties.clear();
	m_properties.fromJson(properties);
}

DcParamCfg::~DcParamCfg()
{
	m_properties.clear();
}

QString DcParamCfg::property(QString name)
{
	return m_properties.get(name);
}

bool DcParamCfg::updateProperty(QString & name, QString & value)
{
	if (name.isEmpty())
		return false;
	if (value.isEmpty())
		return false;

	m_properties.update(name, value);
	update(); // generate update sql and push to sql queue
	return true;
}

QString DcParamCfg::properties()
{
	QString prop;
	m_properties.toJson(prop);
	return prop;
}

void DcParamCfg::updatedb(void)
{
	update();
}

void DcParamCfg::update()
{
	QString updateStrTmp = "UPDATE cfg_parameters SET name = '%1', properties = '%2', value = '%3' WHERE addr = %4 and param = %5;";
	
	QString properties;
	m_properties.toJson(properties);
	QString updateStr = updateStrTmp.arg(name(), properties, value(), QString::number(addr()), QString::number(param()));
	gDbManager.execute(uid(), updateStr);
}

void DcParamCfg::insert()
{
	QString insertStrTmp = "INSERT INTO cfg_parameters(addr, param, name, data_type, \
	flags, value, properties) VALUES(%1, %2, '%3', %4, %5, '%6', '%7');";
	
	QString properties;
	m_properties.toJson(properties);
	QString insertStr = insertStrTmp.arg(QString::number(addr()), QString::number(param()), 
		name(), QString::number(type()), QString::number(flags()), value(), properties);
	gDbManager.execute(uid(), insertStr);

}

void DcParamCfg::remove()
{
	//такой функции в текущем понимании системы не должно быть
}
