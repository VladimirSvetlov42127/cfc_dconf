#include "dc_alg_cfc.h"
#include <db/dc_db_manager.h>

DcAlgCfc::DcAlgCfc(int32_t controllerid, int32_t algid, int32_t position, const QString &name, const QString &properties)
    : DcAlg(algid, position, name)
    , DcDbWrapper(controllerid)
{
	m_ios = new DcPoolSingleKey<DcAlgIOCfc*>();
	m_properties.clear();
    m_properties.fromJson(properties);
}

DcAlgCfc::~DcAlgCfc()
{
	m_properties.clear();
	m_ios->clear();
}

DcPoolSingleKey<DcAlgIOCfc*>* DcAlgCfc::ios()
{
	return m_ios;
}

QString DcAlgCfc::property(QString name)
{
	return m_properties.get(name);
}

QString DcAlgCfc::properties() {
	QString properties;
	m_properties.toJson(properties);
	return properties;
}

bool DcAlgCfc::updateProperty(const QString & name, const QString & value)
{
	if (name.isEmpty())
		return false;
	if (value.isEmpty())
		return false;

	m_properties.update(name, value);
	update(); // generate update sql and push to sql queue
	return true;
}

void DcAlgCfc::updatedb() {
	update();
}

void DcAlgCfc::update()
{
	QString updateStrTmp = "UPDATE algs_cfc SET name = '%1', properties = '%2' WHERE cfc_alg_id = %3;";
	QString properties;
	m_properties.toJson(properties);
	QString updateStr = updateStrTmp.arg(name(), properties, QString::number(index()));
	gDbManager.execute(uid(), updateStr);
}

void DcAlgCfc::insert()
{
	QString insertStrTmp = "INSERT INTO algs_cfc(cfc_alg_id, cfc_position, name, properties) VALUES(%1, %2, '%3', '%4');";
	QString properties;
	m_properties.toJson(properties);
	QString insertStr = insertStrTmp.arg(QString::number(index()), QString::number(position()), name(), properties);
	gDbManager.execute(uid(), insertStr);
}

void DcAlgCfc::remove()
{
	QString removeStrTmp = "DELETE FROM algs_cfc WHERE cfc_alg_id = %1;";
	QString removeStr = removeStrTmp.arg(QString::number(index()));
	gDbManager.execute(uid(), removeStr);
}
