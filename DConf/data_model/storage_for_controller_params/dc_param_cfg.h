#pragma once
#include <data_model/storage_for_controller_params/dc_param.h> 
#include <db/dc_db_wrapper.h>
#include <data_model/dc_properties.h>

class DcParamCfg : public DcParam, public DcDbWrapper
{
public:
    DcParamCfg(int32_t controllerid, int32_t addr, int32_t param, const QString &name,
               DefParamDataType type, attribType_t flags, const QString &value, const QString &properties);
	~DcParamCfg();

	virtual QString property(QString name);
	virtual bool updateProperty(QString &name, QString &value);
	virtual QString properties();
	
	virtual void updatedb(void);

	virtual void update();
	virtual void insert();
	virtual void remove();

private:
	DcProperties m_properties;	// Что делают эти свойства?
};
