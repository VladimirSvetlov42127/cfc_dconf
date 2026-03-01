#include "LineParamEditor.h"

#include <cmath>
#include <qboxlayout.h>
#include <qdebug.h>

#include <dpc/gui/widgets/SpinBox.h>
#include <dpc/gui/widgets/LineEdit.h>

#include <data_model/storage_for_controller_params/DcParamCfg_v2.h>

using namespace Dpc::Gui;

namespace {

} // namespcae

LineParamEditor::LineParamEditor(QWidget *parent) :
	ParamEditor(parent)
{
	init(DEF_DATA_TYPE_STRING);
}

LineParamEditor::LineParamEditor(int type, QWidget *parent) :
	ParamEditor(parent)
{
	init(type);
}

LineParamEditor::LineParamEditor(DcParam *param, QWidget *parent) :
	ParamEditor(param, parent)
{
	init(param->type());
}

void LineParamEditor::setReadOnly(bool readOnly)
{
	if (m_lineEdit)
		m_lineEdit->setReadOnly(readOnly);

	if (m_intSpinBox)
		m_intSpinBox->setReadOnly(readOnly);

	if (m_uintSpinBox)
		m_uintSpinBox->setReadOnly(readOnly);

	if (m_doubleSpinBox)
		m_doubleSpinBox->setReadOnly(readOnly);
}

void LineParamEditor::setValue(const QString &value)
{
	if (m_lineEdit) {
		m_lineEdit->setText(value);
		onValueChanged(value);
	}

	if (m_intSpinBox)
		m_intSpinBox->setValue(value.toInt());

	if (m_uintSpinBox)
		m_uintSpinBox->setValue(value.toUInt());

	if (m_doubleSpinBox)
		m_doubleSpinBox->setValue(value.toDouble());
}

void LineParamEditor::setMinimum(double min)
{
	double m_min;

	auto t = param()->type();
	switch (t) {
	case DEF_DATA_TYPE_BOOL:
	case DEF_DATA_TYPE_BYTEBOOL:
	case DEF_DATA_TYPE_BYTE:
	case DEF_DATA_TYPE_T_8BIT:
	case DEF_DATA_TYPE_WORD:
	case DEF_DATA_TYPE_T_16BIT:
		m_min = std::max<int>(min, 0);
		break;
	case DEF_DATA_TYPE_UNIT:
	case DEF_DATA_TYPE_DWORD:
	case DEF_DATA_TYPE_T_32BIT:
		m_min = std::trunc(std::max<double>(min, 0));
		break;		
	case DEF_DATA_TYPE_CHAR:
	case DEF_DATA_TYPE_T_SBYTE:
		m_min = std::max<int>(min, CHAR_MIN);
		break;
	case DEF_DATA_TYPE_SHORT:
		m_min = std::max<int>(min, SHRT_MIN);
		break;
	case DEF_DATA_TYPE_INTEGER:
	case DEF_DATA_TYPE_LONG:
		m_min = std::max<int>(min, INT_MIN);
		break;
	case DEF_DATA_TYPE_FLOAT:
		m_min = std::max<double>(min, std::numeric_limits<float>::lowest());
		break;
	default:
		break;
	}

	if (m_intSpinBox)
		m_intSpinBox->setMinimum(m_min);

	if (m_uintSpinBox)
		m_uintSpinBox->setMinimum(m_min);

	if (m_doubleSpinBox)
		m_doubleSpinBox->setMinimum(m_min);
}

void LineParamEditor::setMaximum(double max)
{
	double m_max;

	auto t = param()->type();
	switch (t) {
	case DEF_DATA_TYPE_BOOL:
	case DEF_DATA_TYPE_BYTEBOOL:
		m_max = std::min<int>(max, 1);
		break;
	case DEF_DATA_TYPE_BYTE:
	case DEF_DATA_TYPE_T_8BIT:
		m_max = std::min<int>(max, UCHAR_MAX);
		break;
	case DEF_DATA_TYPE_WORD:
	case DEF_DATA_TYPE_T_16BIT:
		m_max = std::min<int>(max, USHRT_MAX);
		break;
	case DEF_DATA_TYPE_UNIT:
	case DEF_DATA_TYPE_DWORD:
	case DEF_DATA_TYPE_T_32BIT:
		m_max = std::trunc(std::min<double>(max, UINT_MAX));
		break;
	case DEF_DATA_TYPE_CHAR:
	case DEF_DATA_TYPE_T_SBYTE:
		m_max = std::min<int>(max, CHAR_MAX);
		break;
	case DEF_DATA_TYPE_SHORT:
		m_max = std::min<int>(max, SHRT_MAX);
		break;
	case DEF_DATA_TYPE_INTEGER:
	case DEF_DATA_TYPE_LONG:
		m_max = std::min<int>(max, INT_MAX);
		break;
	case DEF_DATA_TYPE_FLOAT:
		m_max = std::min<double>(max, std::numeric_limits<float>::max());
		break;
	default:
		break;
	}

	if (m_intSpinBox)
		m_intSpinBox->setMaximum(m_max);

	if (m_uintSpinBox)
		m_uintSpinBox->setMaximum(m_max);

	if (m_doubleSpinBox)
		m_doubleSpinBox->setMaximum(m_max);
}

void LineParamEditor::setRange(double bottom, double top)
{
	setMinimum(bottom);
	setMaximum(top);
}

void LineParamEditor::setDecimals(int decimals)
{
	if (!m_doubleSpinBox)
		return;

	m_doubleSpinBox->blockSignals(true);
	m_doubleSpinBox->setDecimals(decimals);
	m_doubleSpinBox->setValue(param()->value().toDouble());
	m_doubleSpinBox->blockSignals(false);
}

void LineParamEditor::setLength(int len)
{
	if (m_lineEdit)
		m_lineEdit->setMaxLength(len);
}

void LineParamEditor::setSpecialValueText(const QString &text)
{
	if (m_intSpinBox)
		m_intSpinBox->setSpecialValueText(text);

	if (m_uintSpinBox)
		m_uintSpinBox->setSpecialValueText(text);

	if (m_doubleSpinBox)
		m_doubleSpinBox->setSpecialValueText(text);
}

void LineParamEditor::addValueText(double value, const QString & text)
{
	if (m_intSpinBox)
		m_intSpinBox->addValueText(value, text);

	if (m_uintSpinBox)
		m_uintSpinBox->addValueText(value, text);

	if (m_doubleSpinBox)
		m_doubleSpinBox->addValueText(value, text);
}

void LineParamEditor::onValueChanged(int value)
{
	setParamValue(QString::number(value));
}

void LineParamEditor::onValueChanged(uint value)
{
	setParamValue(QString::number(value));
}

void LineParamEditor::onValueChanged(double value)
{
	setParamValue(QString::number(value, 'f', m_doubleSpinBox->decimals()));
}

void LineParamEditor::onValueChanged(const QString &text)
{
	setParamValue(text);
}

void LineParamEditor::init(int type)
{
	m_lineEdit			= nullptr;
	m_intSpinBox		= nullptr;
	m_uintSpinBox		= nullptr;
	m_doubleSpinBox		= nullptr;
	QWidget *editWidget = nullptr;
	switch (type)
	{
	case DEF_DATA_TYPE_BYTE:
	case DEF_DATA_TYPE_BYTEBOOL:
	case DEF_DATA_TYPE_WORD:
	case DEF_DATA_TYPE_BOOL:
	case DEF_DATA_TYPE_T_SBYTE:
	case DEF_DATA_TYPE_T_8BIT:
	case DEF_DATA_TYPE_T_16BIT:
	case DEF_DATA_TYPE_CHAR:
	case DEF_DATA_TYPE_SHORT:
	case DEF_DATA_TYPE_INTEGER:
	case DEF_DATA_TYPE_LONG:
		editWidget = makeIntSpinBox(type);
		break;
	case DEF_DATA_TYPE_UNIT:
	case DEF_DATA_TYPE_DWORD:
	case DEF_DATA_TYPE_T_32BIT:
		editWidget = makeUIntSpinBox();
		break;
	case DEF_DATA_TYPE_FLOAT:
		editWidget = makeDoubleSpinBox();
		break;
	case DEF_DATA_TYPE_DATETIME:
	case DEF_DATA_TYPE_UTC:
	case DEF_DATA_TYPE_OUI:
	case DEF_DATA_TYPE_IP4ADR:
	case DEF_DATA_TYPE_STRING:
	default:
		editWidget = makeLineEdit(type);
		break;
	}

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(editWidget);
}

IntSpinBox * LineParamEditor::makeIntSpinBox(int type)
{	
	int m_min = INT_MIN;
	int m_max = INT_MAX;
	switch (type) {
	case DEF_DATA_TYPE_BOOL:
	case DEF_DATA_TYPE_BYTEBOOL:
		m_min = 0; m_max = 1; break;
	case DEF_DATA_TYPE_BYTE:
	case DEF_DATA_TYPE_T_8BIT:
		m_min = 0; m_max = UCHAR_MAX; break;
	case DEF_DATA_TYPE_WORD:
	case DEF_DATA_TYPE_T_16BIT:
		m_min = 0; m_max = USHRT_MAX; break;
	case DEF_DATA_TYPE_CHAR:
	case DEF_DATA_TYPE_T_SBYTE:
		m_min = CHAR_MIN; m_max = CHAR_MAX; break;
	case DEF_DATA_TYPE_SHORT:
		m_min = SHRT_MIN; m_max = SHRT_MAX; break;
	default: break;
	}
	
	m_intSpinBox = new IntSpinBox;
	m_intSpinBox->setRange(m_min, m_max);
	if (param())
		m_intSpinBox->setValue(param()->value().toInt());

	connect(m_intSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	return m_intSpinBox;
}

UIntSpinBox * LineParamEditor::makeUIntSpinBox()
{
	m_uintSpinBox = new UIntSpinBox;
	m_uintSpinBox->setRange(0, UINT_MAX);
	if (param())
		m_uintSpinBox->setValue(param()->value().toUInt());

	connect(m_uintSpinBox, SIGNAL(valueChanged(uint)), this, SLOT(onValueChanged(uint)));
	return m_uintSpinBox;
}

DoubleSpinBox * LineParamEditor::makeDoubleSpinBox()
{
	m_doubleSpinBox = new DoubleSpinBox;
	m_doubleSpinBox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
	m_doubleSpinBox->setValue(param()->value().toDouble());

	connect(m_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
	return m_doubleSpinBox;
}

LineEdit * LineParamEditor::makeLineEdit(int type)
{
	auto t = LineEdit::StringType;	
	if (type == DEF_DATA_TYPE_IP4ADR)
		t = LineEdit::IpAddressType;
	if (type == DEF_DATA_TYPE_OUI)
		t = LineEdit::MacAddressType;

	m_lineEdit = new LineEdit(t);
	if (param()) {
		m_lineEdit->setText(param()->value());
		if (t == LineEdit::StringType)
			m_lineEdit->setMaxLength(dynamic_cast<DcParamCfg_v2*>(param())->getDataLenInBytes());		
	}

	connect(m_lineEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(onValueChanged(const QString &)));
	return m_lineEdit;
}
