#include "dc_properties.h"

#include <QJsonDocument>
#include <QJsonObject>

DcProperties::DcProperties()
{

}

DcProperties::~DcProperties()
{
}

bool DcProperties::operator==(const DcProperties & other) const
{
	return m_properties == other.m_properties;
}

bool DcProperties::add(const QString & name, const QString & value)
{
	if (name.isEmpty())
		return false;

	if (value.isEmpty())
		return false;

	if (exist(name))
		return update(name, value);

	m_properties.insert(name, value);
	return true;
}

bool DcProperties::exist(const QString & name)
{
	auto it = m_properties.find(name);
	if (it == m_properties.end())
		return false;
	else
		return true;
}

bool DcProperties::update(const QString & name, const QString & value)
{
	if (value.isEmpty())
		return false;

	if (!exist(name))
		return add(name, value);

	m_properties[name] = value;
	return true;
}

bool DcProperties::remove(const QString & name)
{
	auto it = m_properties.find(name);
	if (it == m_properties.end())
		return false;

	m_properties.erase(it);
	return true;
}

bool DcProperties::fromJson(const QString & jsondoc)
{
	if (jsondoc.isEmpty())
		return true;

	/*Document document;
	document.Parse(jsondoc.toStdString().c_str());
	//DEPT обработать ошибки некорректного json файла
	for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
		if (!itr->value.IsString())
			continue;
		QString propname = itr->name.GetString();
		QString propvalue = itr->value.GetString();
		m_properties.insert(propname, propvalue);
	}*/ //TODO realization on Qt json

	QJsonDocument json = QJsonDocument::fromJson(jsondoc.toUtf8());
	if (json.isNull())
		return false;

	QJsonObject obj = json.object();
	for (auto it = obj.begin(); it != obj.end(); it++) {
		QString value;
		if (it.value().isBool())
			value = QString::number(it.value().toBool());
		else if (it.value().isDouble())
			value = QString::number(it.value().toDouble());
		else
			value = it.value().toString();

		m_properties.insert(it.key(), value);
	}

	return true;
}

bool DcProperties::toJson(QString & result)
{
	if (m_properties.empty())
		return true;

	result = "{";
	QMapIterator<QString, QString> i(m_properties);
	while (i.hasNext()) {
		i.next();
		QString tmp = "\"%1\":\"%2\"";
		result += tmp.arg(i.key(), i.value());
		if (i.hasNext())
			result += ",";
	}
	result += "}";
	return true;
}

void DcProperties::clear(void)
{
	if (m_properties.empty())
		return;
	//qDeleteAll(m_properties);
	m_properties.clear();
}

QString DcProperties::get(QString name) const{
	if (!m_properties.contains(name))
		return QString();
	else
		return m_properties.value(name);;
}

void DcProperties::copyTo(DcProperties *properties) {
	for (auto it = m_properties.begin(); it != m_properties.end(); it++) {
		if (properties->exist(it.key()))
			continue;
		properties->add(it.key(), it.value());
	}
}
