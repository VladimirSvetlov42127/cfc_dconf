#pragma once
#include "dc_alg.h"
#include <db/dc_db_wrapper.h>
#include "dc_pool.h"
#include "dc_alg_io_internal.h"
#include <data_model/dc_properties.h>

class DcAlgInternal : public DcAlg, public DcDbWrapper {

public:
    DcAlgInternal(int32_t controllerid, int32_t algid, int32_t position, const QString &name, const QString &properties);
	~DcAlgInternal();
	
	DcPoolSingleKey<DcAlgIOInternal*> *ios();
	QString property(QString name) const;
	QString properties();
	bool updateProperty(QString &name, QString &value);
	void updatedb();

	bool operator==(const DcAlgInternal &other) const;

private:
	DcPoolSingleKey<DcAlgIOInternal*> *m_ios; //inputs/outputs
	DcProperties m_properties;

public:
	void update();
	void insert();
	void remove();
};
