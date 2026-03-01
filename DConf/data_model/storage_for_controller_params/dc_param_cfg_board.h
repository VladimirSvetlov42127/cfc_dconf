#pragma once
#include <data_model/storage_for_controller_params/dc_param.h> 
#include <db/dc_db_wrapper.h>
#include <data_model/dc_properties.h>

class DcParamCfgBoard : public DcParam, public DcDbWrapper
{
public:
    DcParamCfgBoard(int32_t controllerid, int32_t board, int32_t addr, int32_t param, const QString &name, DefParamDataType type,
                    uint8_t flags, const QString &value, const QString &properties);
	~DcParamCfgBoard();

	QString properties();
	QString property(QString name);
	bool updateProperty(QString &name, QString &value);

	int32_t board();
	void updatedb(void);

	void update();
	void insert();
	void remove();

private:
	int32_t m_board;
	DcProperties m_properties;
};
