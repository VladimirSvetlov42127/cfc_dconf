#include "ParamEditor.h"

ParamEditor::ParamEditor(QWidget *parent) :
	QWidget(parent),
	m_param(nullptr)
{
}

ParamEditor::ParamEditor(DcParam *param, QWidget *parent) : 
	QWidget(parent),
	m_param(param)
{
}

ParamEditor::~ParamEditor()
{
}

DcParam* ParamEditor::param() const
{ 
	return m_param; 
}

void ParamEditor::setReadOnly(bool readOnly)
{
	setEnabled(!readOnly);
}

void ParamEditor::setParamValue(const QString & value)
{
	if (!m_param) {
		if (m_value != value) {
			auto prevValue = m_value;
			m_value = value;
			emit valueChanged(value, prevValue);			
		}
		return;
	}

	auto t = param()->type();
	if (t != DEF_DATA_TYPE_STRING && value.trimmed().isEmpty())
		return;

	QString oldValue = param()->value();
	bool ok = false;

	switch (t)
	{
	case DEF_DATA_TYPE_BYTE:
	case DEF_DATA_TYPE_BYTEBOOL:
	case DEF_DATA_TYPE_WORD:
	case DEF_DATA_TYPE_BOOL:	
	case DEF_DATA_TYPE_CHAR:
	case DEF_DATA_TYPE_T_8BIT:
	case DEF_DATA_TYPE_T_16BIT:	
	case DEF_DATA_TYPE_T_SBYTE:
	case DEF_DATA_TYPE_SHORT:
	case DEF_DATA_TYPE_INTEGER:
	case DEF_DATA_TYPE_LONG:
		if (oldValue.toInt(&ok) == value.toInt() && ok)
			return;
		break;
	case DEF_DATA_TYPE_UNIT:
	case DEF_DATA_TYPE_T_32BIT:
	case DEF_DATA_TYPE_DWORD:
	case DEF_DATA_TYPE_DATETIME:
		if (oldValue.toUInt(&ok) == value.toUInt() && ok)
			return;
		break;
	case DEF_DATA_TYPE_FLOAT:
		if (oldValue.toFloat(&ok) == value.toFloat() && ok)
			return;
		break;
	case DEF_DATA_TYPE_UTC:
		if (oldValue.toLongLong(&ok) == value.toLongLong() && ok) return;
		break;
	case DEF_DATA_TYPE_OUI:			// mac address
	case DEF_DATA_TYPE_IP4ADR:
	case DEF_DATA_TYPE_STRING:
	default:
		if (oldValue == value) return;
		break;
	}

	param()->updateValue(value);
	emit valueChanged(param()->value(), oldValue);
}