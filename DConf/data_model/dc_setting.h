#pragma once
#include <QString>
#include <db/dc_db_wrapper.h>

class DcSetting : public DcDbWrapper {

public:
    DcSetting(int32_t controllerid, const QString &name, const QString &value);
	~DcSetting();

private:
	QString m_name;
	QString m_value;

public:
	QString name();
	QString value();
	
	bool updateValue(const QString & value);
	
public:
	void update();
	void insert();
	void remove();
};
