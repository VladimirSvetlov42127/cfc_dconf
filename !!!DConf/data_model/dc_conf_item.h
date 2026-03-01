#pragma once
#include <qstring.h>
#include <list>
#include <data_model/dc_properties.h>

class DcConfItem {
public:
	DcConfItem(int32_t id, int32_t parent_id, const QString &type, const QString &name, const QString &properties);
	~DcConfItem();

private:
	int32_t m_id;
	int32_t m_parent_id;
	QString m_type;
	QString m_name;
	//QString m_properties;
	DcProperties *m_properties;

public:
	//getters
	int32_t id();
	int32_t parent_id();
	QString type();
	QString name();
	QString property(QString name);
	DcProperties *properties();
	//setters
	void updateName(QString &name);
	void updateProperty(QString name, QString value);

};
