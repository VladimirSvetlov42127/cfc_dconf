#include "dc_alg_internal.h"
#include <db/dc_db_manager.h>

DcAlgInternal::DcAlgInternal(int32_t controllerid, int32_t algid, int32_t position, const QString &name, const QString &properties)
    : DcAlg(algid, position, name)
    , DcDbWrapper(controllerid)
{
	m_ios = new DcPoolSingleKey<DcAlgIOInternal*>();
	m_properties.clear();
	m_properties.fromJson(properties);
}

DcAlgInternal::~DcAlgInternal()
{
	m_properties.clear();
	m_ios->clear();
}

DcPoolSingleKey<DcAlgIOInternal*>* DcAlgInternal::ios()
{
	return m_ios;
}

QString DcAlgInternal::property(QString name) const
{
	return m_properties.get(name);
}

QString DcAlgInternal::properties()
{
	QString prop;
	m_properties.toJson(prop);
	return prop;
}

void DcAlgInternal::updatedb() {
	update();
}

bool DcAlgInternal::operator==(const DcAlgInternal & other) const
{
	if (position() == other.position() && 
		name().trimmed().toUpper() == other.name().trimmed().toUpper() && 
		m_properties == other.m_properties)
		return true;

	return false;
}

bool DcAlgInternal::updateProperty(QString & name, QString & value)
{
	if (name.isEmpty())
		return false;
	if (value.isEmpty())
		return false;

	m_properties.update(name, value);
	update(); // generate update sql and push to sql queue
	return true;
}

void DcAlgInternal::update()
{
	QString updateStrTmp = "UPDATE algs SET name = '%1', properties = '%2' WHERE alg_id = %3;";
	QString properties;
	m_properties.toJson(properties);
	QString updateStr = updateStrTmp.arg(name(), properties, QString::number(index()));
	gDbManager.execute(uid(), updateStr);
}

void DcAlgInternal::insert()
{
	QString insertStrTmp = "INSERT INTO algs(alg_id, position, name, properties) VALUES(%1, %2, '%3', '%4');";
	QString properties;
	m_properties.toJson(properties);
	QString insertStr = insertStrTmp.arg(QString::number(index()), QString::number(position()), name(), properties);
	gDbManager.execute(uid(), insertStr);
}

void DcAlgInternal::remove()
{
	m_ios->removeAll();
	QString removeStrTmp = "DELETE FROM algs WHERE alg_id = %1;";
	QString removeStr = removeStrTmp.arg(QString::number(index()));
	gDbManager.execute(uid(), removeStr);
}
