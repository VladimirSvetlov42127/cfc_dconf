#include "dc_param.h"

DcParam::DcParam(int32_t addr, int32_t param, QString name, DefParamDataType type, 
	attribType_t flags, QString value) :
	m_addr(addr), 
	m_param(param), 
	m_name(name), 
	m_type(type), 
	m_flags(flags), 
	m_value(value),
	was_changed_bool(false)
{
}

int32_t DcParam::addr()
{
	return m_addr;
}

int32_t DcParam::param()
{
	return m_param;
}

QString DcParam::name()
{
	return m_name;
}

DefParamDataType DcParam::type()
{
	return m_type;
}

attribType_t DcParam::flags()
{
	return m_flags;
}

QString DcParam::value()
{
	return m_value;
}

bool DcParam::was_changed() {
	return was_changed_bool;
}

void DcParam::reset_param_changes() {
	was_changed_bool = false;
}

bool DcParam::updateValue(QString newval)
{
	if (newval.isNull())
		return false;
	//if (newval.isEmpty())
	//	return false;
	if (newval.compare(m_value) == 0)
		return true;

	if (m_value.compare(newval) == 0)
		return true;
	m_value = newval;
	was_changed_bool = true;				// Значение изменилось!
	updatedb();
	return true;
}

bool DcParam::updateName(QString newname)
{
	if (newname.isNull())
		return false;
	if (newname.isEmpty())
		return false;
	if (newname.compare(m_name) == 0)
		return true;

	was_changed_bool = true;
	updatedb();
	return true;
}
