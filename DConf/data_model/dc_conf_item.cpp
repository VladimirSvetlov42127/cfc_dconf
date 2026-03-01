#include "dc_conf_item.h"
//#include <db/dc_db_manager.h>

DcConfItem::DcConfItem(int32_t id, int32_t parent_id, const QString &type, const QString &name, const QString &properties) :
	m_id(id),
	m_parent_id(parent_id),
	m_type(type),
	m_name(name)
{
	m_properties = new DcProperties();
	m_properties->fromJson(properties);
}

DcConfItem::~DcConfItem()
{
}

//getters
int32_t DcConfItem::id() {
	return m_id;
}

int32_t DcConfItem::parent_id() {
	return m_parent_id;
}

QString DcConfItem::type() {
	return m_type;
}

QString DcConfItem::name() {
	return m_name;
}

QString DcConfItem::property(QString name) {
	return m_properties->get(name);
}

DcProperties * DcConfItem::properties()
{
	return m_properties;
}


//setters
void DcConfItem::updateName(QString &name) {
	if (name.isEmpty())
		return;
	m_name = name;
	return;
}

void DcConfItem::updateProperty(QString name, QString value) {
	if (name.isEmpty() || value.isEmpty())
		return;
	m_properties->update(name, value);
}
