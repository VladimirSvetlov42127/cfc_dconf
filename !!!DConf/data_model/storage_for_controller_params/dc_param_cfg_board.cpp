#include "dc_param_cfg_board.h"
#include <db/dc_db_manager.h>

DcParamCfgBoard::DcParamCfgBoard(int32_t controllerid, int32_t board, int32_t addr, int32_t param, const QString &name,
    DefParamDataType type, uint8_t flags, const QString &value, const QString &properties)
    : DcParam(addr, param, name, type, flags, value)
    , DcDbWrapper(controllerid)
    , m_board(board)
{
	m_properties.clear();
	m_properties.fromJson(properties);
}

DcParamCfgBoard::~DcParamCfgBoard()
{
	m_properties.clear();
}

QString DcParamCfgBoard::properties()
{
	QString prop;
	m_properties.toJson(prop);
	return prop;
}

QString DcParamCfgBoard::property(QString name)
{
	return m_properties.get(name);
}

bool DcParamCfgBoard::updateProperty(QString & name, QString & value)
{
	if (name.isEmpty())
		return false;
	if (value.isEmpty())
		return false;

	m_properties.update(name, value);
	update(); // generate update sql and push to sql queue
	return true;
}

void DcParamCfgBoard::updatedb(void)
{
	update();
}

int32_t DcParamCfgBoard::board() {
	return m_board;
}

void DcParamCfgBoard::update()
{
	QString updateStrTmp = "UPDATE board_cfg_parameters SET name = '%1', properties = '%2', value = '%3' \
	WHERE board_id = %4 and addr = %5 and param = %6;";

	QString properties;
	m_properties.toJson(properties);
	QString updateStr = updateStrTmp.arg(name(), properties, value(), QString::number(board()), QString::number(addr()), QString::number(param()));
	gDbManager.execute(uid(), updateStr);
}

void DcParamCfgBoard::insert()
{
	QString insertStrTmp = "INSERT INTO board_cfg_parameters(board_id, addr, param, name, data_type, \
	flags, value, properties) VALUES(%1, %2, %3, '%4', %5, '%6', '%7', '%8');";

	QString properties;
	m_properties.toJson(properties);
	QString insertStr = insertStrTmp.arg(QString::number(board()), QString::number(addr()), QString::number(param()),
		name(), QString::number(type()), getAttributeStringFromInt(flags()), value(), properties);
	gDbManager.execute(uid(), insertStr);
}

void DcParamCfgBoard::remove()
{
    QString removeStrTmp = "DELETE FROM board_cfg_parameters WHERE board_id = %1 and addr = %2 and param = %3;";
    QString removeStr = removeStrTmp.arg(board()).arg(addr()).arg(param());
    gDbManager.execute(uid(), removeStr);
}
