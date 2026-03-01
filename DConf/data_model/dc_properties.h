#pragma once
#include <qmap.h>
#include <qstring.h>
#include <qpair.h>

class DcProperties {
public:
	DcProperties();
	~DcProperties();

	bool operator==(const DcProperties &other) const;

	QMap<QString, QString> props() const { return m_properties; }

private:
	QMap<QString, QString> m_properties;

public:
	bool add(const QString &name, const QString &value);
	bool exist(const QString &name);
	bool update(const QString &name, const QString &value);
	bool remove(const QString &name);
	bool fromJson(const QString &jsondoc);
	bool toJson(QString &result);
	void clear(void);
	QString get(QString name) const;
	void copyTo(DcProperties *properties);
};
