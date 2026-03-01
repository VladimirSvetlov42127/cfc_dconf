#pragma once
#include <stdint.h>
#include "dc_alg.h"
#include <db/dc_db_wrapper.h>
#include "dc_pool.h"
#include "dc_alg_io_cfc.h"
#include <data_model/dc_properties.h>

class DcAlgCfc : public DcAlg, public DcDbWrapper {

public:
    DcAlgCfc(int32_t controllerid, int32_t algid, int32_t position, const QString &name, const QString &properties);
	~DcAlgCfc();

	DcPoolSingleKey<DcAlgIOCfc*> *ios();
	QString property(QString name);
	QString properties();
	bool updateProperty(const QString &name, const QString &value);
	void updatedb();

private:
	DcPoolSingleKey<DcAlgIOCfc*> *m_ios; //inputs/outputs
	DcProperties m_properties;

public:
	void update();
	void insert();
	void remove();
};
