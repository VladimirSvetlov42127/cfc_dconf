#include "DcParamCfg_v2.h"
#include <data_model/storage_for_controller_params/DcController_v2.h>
#include <data_model/dc_data_manager.h>
#include <db/dc_db_manager.h>

#include <qsharedpointer.h>

DcParamCfg_v2::DcParamCfg_v2(int32_t controllerid, const QString &name, uint32_t addr, uint32_t ind, const QString &value, QSharedPointer<DcParamCfg_v2_Headline> headline)
    : DcParamCfg(controllerid, int32_t(), int32_t(), QString(), DefParamDataType::DEF_DATA_TYPE_BOOL, attribType_t(), QString(), QString()) // Создаем пустое
    , m_name(name)
    , m_address(addr)
    , m_index(ind)
    , m_value(value)
    , m_wasChanged(false)
{
	if (headline) {
		m_headline = headline;
	}
	else  {// получение через контроллер
        DcController_v2 *controller = dynamic_cast<DcController_v2*> (gDataManager.controllers()->getById(controllerid));
		if (controller) {
			for(QSharedPointer<DcParamCfg_v2_Headline> headline: *controller->headlines()) {
				if (headline->getAddress() == m_address) {
					m_headline = headline;
					break;
				}
			}
		}		
	}
}


DcParamCfg_v2::~DcParamCfg_v2()
{
}

int32_t DcParamCfg_v2::addr()
{
	return m_address;
}

int32_t DcParamCfg_v2::param()
{
	return m_index;
}

QString DcParamCfg_v2::name()
{
	return m_name;
}

DefParamDataType DcParamCfg_v2::type()
{
	if (m_headline) return m_headline->getDataType();
	else return DEF_DATA_TYPE_UNDEF;
}

attribType_t DcParamCfg_v2::flags()
{
	if (m_headline) return m_headline->getAttributes();
	else return attribType_t();
}

QString DcParamCfg_v2::value()
{
	return m_value;
}

bool DcParamCfg_v2::was_changed()
{
	return m_wasChanged;
}

void DcParamCfg_v2::reset_param_changes()
{
	m_wasChanged = false;
}

bool DcParamCfg_v2::updateValue(QString newval)
{
	if (newval.isNull())
		newval = "";
	if (newval == m_value)
		return true;

	m_value = newval;
	m_wasChanged = true;				// Значение изменилось!
	updatedb();
	emit m_helper.valueChanged(m_value);
	return true;
}

bool DcParamCfg_v2::updateName(QString newname)
{
	if (newname.isNull())
		return false;
	if (newname.isEmpty())
		return false;
	if (newname.compare(m_name) == 0)
		return true;

	m_wasChanged = true;
	updatedb();
	return true;
}

QString DcParamCfg_v2::property(QString name)
{
	return QString();
}

bool DcParamCfg_v2::updateProperty(QString & name, QString & value)
{
	return true;
}

void DcParamCfg_v2::updatedb(void)
{
	update();
}

void DcParamCfg_v2::update()
{
	QString updateStrTmp = "UPDATE cfg_parameters_items SET name = '%1', value = '%2' WHERE address = %3 and param_index = %4;";

	//QString properties;	// Пока не поддерживаем
	//m_properties.toJson(properties);
	int address = addr();
	int ind = param();
	QString updateStr = updateStrTmp.arg(name(), value(), QString::number(addr()), QString::number(param()));
	gDbManager.execute(uid(), updateStr);
}

void DcParamCfg_v2::insert()	// Хотя она нигде не вызывается
{
	QString insertStrTmp = "INSERT INTO cfg_parameters_items(name, address, param_index, value) VALUES('%1', %2, %3, '%4');";

	//QString properties;
	//m_properties.toJson(properties);
	QString insertStr = insertStrTmp.arg(name(), QString::number(addr()), QString::number(param()), value());
	gDbManager.execute(uid(), insertStr);
}

void DcParamCfg_v2::remove()
{
	QString deleteStrTmp = "DELETE FROM cfg_parameters_items WHERE address = %1 and param_index = %2;";

	//QString properties;
	//m_properties.toJson(properties);
	QString deleteStr = deleteStrTmp.arg(addr()).arg(param());
	gDbManager.execute(uid(), deleteStr);
}

uint16_t DcParamCfg_v2::getProfileCount()
{
	return m_headline->getProfileCount();
}

uint16_t DcParamCfg_v2::getSubProfileCount()
{
	return m_headline->getSubProfileCount();
}

uint16_t DcParamCfg_v2::getDataLenInBytes()
{
	return m_headline->getDataLenInBytes();
}
