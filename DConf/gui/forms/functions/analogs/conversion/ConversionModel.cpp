#include "ConversionModel.h"

#include <data_model/storage_for_controller_params/DcController_v2.h>

namespace {
	const uint16_t CUSTOM_CONVERSION_TYPE_VALUE = 3;
}

ConversionModel::ConversionModel(DcController* contr, QObject* parent) :
	QAbstractTableModel(parent),
	m_contr(contr)
{
	auto param = dynamic_cast<DcParamCfg_v2*>(contr->params_cfg()->get(SP_AIN_CONVERS_WORDAIN_PARAM, 0));
	if (!param)
		return;

	for (int i = 0; i < param->getSubProfileCount(); i++) {
		QVariantList data;
		data.append(i + 1);
		data.append(contr->getValue(SP_AIN_CONVERS_WORDAIN_PARAM, i).toUInt());
		data.append(contr->getValue(SP_AIN_CONVERS_WORDOUT_PARAM, i).toUInt());
		data.append(contr->getValue(SP_AIN_CONVERS_BYTE_PARAM, i).toUInt());

		m_items.push_back(data);
	}
}

int ConversionModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0;

	return m_items.size();
}

int ConversionModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0;

	return ColumnsCount;
}

QVariant ConversionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return columnName((Columns)section);

	return QVariant();
}

Qt::ItemFlags ConversionModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags fl = QAbstractTableModel::flags(index);

	int col = index.column();
	if (!col)
		return fl;

	if (index.data().isValid())
		fl |= Qt::ItemIsEditable;

	return fl;
}

QVariant ConversionModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	auto data = m_items.at(index.row());
	int col = index.column();

	if (Qt::TextAlignmentRole == role)
		return Qt::AlignCenter;

	if (Qt::EditRole == role || Qt::DisplayRole == role) {
		if (SettingColumn == col)
			return data.value(ConversionTypeColumn).toUInt() == CUSTOM_CONVERSION_TYPE_VALUE ? true : QVariant();
		return data.value(col);
	}

	return QVariant();
}

bool ConversionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	int row = index.row();
	int col = index.column();
	auto& data = m_items[row];

	if (!index.isValid())
		return false;

	if (col < Columns::SettingColumn) {
		data[col] = value;

		if (col == Columns::SourceColumn)
			m_contr->setValue(SP_AIN_CONVERS_WORDAIN_PARAM, row, value);
		if (col == Columns::DestinationColumn)
			m_contr->setValue(SP_AIN_CONVERS_WORDOUT_PARAM, row, value);
		if (col == Columns::ConversionTypeColumn)
			m_contr->setValue(SP_AIN_CONVERS_BYTE_PARAM, row, value);

		emit dataChanged(index, this->index(row, Columns::SettingColumn));
		return true;
	}

	return false;
}

QString ConversionModel::columnName(Columns col) const
{
	switch (col)
	{
	case Columns::NumColumn: return "№";
	case Columns::SourceColumn: return "Исходный аналог";
	case Columns::DestinationColumn: return "Преобразованный аналог";
	case Columns::ConversionTypeColumn: return "Тип преобразования";
	case Columns::SettingColumn: return "Настройка";
	}

	return QString();
}

std::vector<QVariantList> ConversionModel::getItems() const
{
	return m_items;
}