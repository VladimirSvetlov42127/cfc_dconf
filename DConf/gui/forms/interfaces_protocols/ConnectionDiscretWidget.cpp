#include "ConnectionDiscretWidget.h"

#include <qgridlayout.h>
#include <qcombobox.h>
#include <qdebug.h>

#include <gui/forms/DcSignalManager.h>

#include <dpc/gui/dialogs/msg_box/MsgBox.h>

using namespace Dpc::Gui;

namespace {
	const QString TEXT_NO = "Нет";
	const QString TEXT_VIRTUAL_INPUT = "Виртуальный вход";
	const QString TEXT_CONNECTION_DISCRET = "Дискрет связи";

	const uint16_t INVALID_VALUE_BYTE = 0xFF;
	const uint16_t INVALID_VALUE_WORD = 0xFFFF;

	enum {
		SignalRole = Qt::UserRole + 1,
		VirtualNumRole
	};

} // namespace

ConnectionDiscretWidget::ConnectionDiscretWidget(DcController *controller, int interfaceIdx, int deviceIdx, const QString &deviceName, QWidget *parent) :
	QWidget(parent),
	m_controller(controller),
	m_interfaceIdx(interfaceIdx),
	m_deviceIdx(deviceIdx),
	m_deviceName(deviceName),
	m_comboBox(new QComboBox(this))
{
	updateList();

	connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionDiscretWidget::onComboBoxIndexChanged);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(m_comboBox);
	layout->setContentsMargins(0, 0, 0, 0);
}

ConnectionDiscretWidget::~ConnectionDiscretWidget()
{
}

void ConnectionDiscretWidget::updateList()
{
	if (!m_controller)
		return;

	int currentConnectionDiscret = -1;
	int sp = subscriberProfile(m_interfaceIdx, m_deviceIdx);
	if (sp >= 0)
		currentConnectionDiscret = getParam(SP_INDCON_OUTWORD_PARAM, sp)->value().toInt();

	// Получаем список свободных виртуальных дискретных входов
	QList<DcSignal*> freeVirtualDiscretList;
	for (auto &it : DcSignalManager(m_controller).getSignalList(DefSignalDirection::DEF_SIG_DIRECTION_OUTPUT))
		if (it.second.count() && it.second.first()->subtype() == DEF_SIG_SUBTYPE_VIRTUAL && it.second.first()->direction() == DEF_SIG_DIRECTION_INPUT) {
			freeVirtualDiscretList = it.second;
			break;
		}

	// Очищаем и добавляем в комбобокс элемент нет.
	bool block = m_comboBox->blockSignals(true);
	m_comboBox->clear();
	m_comboBox->addItem(TEXT_NO);
	m_previousComboBoxIdx = 0;

	// Добавляем в комбобокс все сводобные виртуальные дискретные входы и виртуальный вход, который указан как дискрет связи с данным устройством.
	int num = 0;
	for (auto s: m_controller->getSignalList(DEF_SIG_TYPE_DISCRETE, DEF_SIG_SUBTYPE_VIRTUAL)) {
		num++;

		if (s->internalId() == currentConnectionDiscret || freeVirtualDiscretList.contains(s)) {
			m_comboBox->addItem(QString("%1 %2").arg(TEXT_VIRTUAL_INPUT).arg(num));

			int lastIdx = m_comboBox->count() - 1;
			m_comboBox->setItemData(lastIdx, QVariant::fromValue((void*)s), SignalRole);
			m_comboBox->setItemData(lastIdx, num, VirtualNumRole);

			if (s->internalId() == currentConnectionDiscret) {
				m_comboBox->setCurrentIndex(lastIdx);
				m_previousComboBoxIdx = lastIdx;
			}
		}
	}

	m_comboBox->blockSignals(block);
}

void ConnectionDiscretWidget::onComboBoxIndexChanged(int idx)
{
	int sp = subscriberProfile(m_interfaceIdx, m_deviceIdx);
	if (sp < 0)
		sp = subscriberProfile(INVALID_VALUE_BYTE, INVALID_VALUE_BYTE);
	if (sp < 0) {
		MsgBox::error(QString("Не осталось слотов под дискрет связи(интерфейс = %1, устройство = %2)").arg(m_interfaceIdx).arg(m_deviceIdx));
		return;
	}

	DcParamCfg *subscriberInterfaceParam = getParam(SP_INDCON_BYTE_PARAM, sp * PROFILE_SIZE);
	DcParamCfg *subscriberDeviceParam = getParam(SP_INDCON_BYTE_PARAM, sp * PROFILE_SIZE + 1);
	DcParamCfg *subscriberConnectionDiscretParam = getParam(SP_INDCON_OUTWORD_PARAM, sp);

	DcSignal* prevSignal = (DcSignal*)m_comboBox->itemData(m_previousComboBoxIdx, SignalRole).value<void*>();
	if (prevSignal) {
		int virtualNum = m_comboBox->itemData(m_previousComboBoxIdx, VirtualNumRole).toInt();
		if (virtualNum)
			prevSignal->updateName(QString("%1 %2").arg(TEXT_VIRTUAL_INPUT).arg(virtualNum));
	}

	DcSignal* newSignal = (DcSignal*)m_comboBox->itemData(idx, SignalRole).value<void*>();
	if (newSignal) {
		newSignal->updateName(QString("%1 c %2").arg(TEXT_CONNECTION_DISCRET).arg(m_deviceName));

		subscriberInterfaceParam->updateValue(QString::number(m_interfaceIdx));
		subscriberDeviceParam->updateValue(QString::number(m_deviceIdx));
		subscriberConnectionDiscretParam->updateValue(QString::number(newSignal->internalId()));
	}
	else {
		subscriberInterfaceParam->updateValue(QString::number(INVALID_VALUE_BYTE));
		subscriberDeviceParam->updateValue(QString::number(INVALID_VALUE_BYTE));
		subscriberConnectionDiscretParam->updateValue(QString::number(INVALID_VALUE_WORD));
	}

	m_previousComboBoxIdx = idx;
}

int ConnectionDiscretWidget::subscriberProfile(int interfaceIdx, int deviceIdx) const
{
	DcParamCfg_v2 *subscriberParam = getParam(SP_INDCON_BYTE_PARAM, 0);
	if (subscriberParam)
		for (int i = 0; i < subscriberParam->getProfileCount(); i++) {
			DcParamCfg *subscriberInterfaceParam = getParam(SP_INDCON_BYTE_PARAM, i * PROFILE_SIZE);
			DcParamCfg *subscriberDeviceParam = getParam(SP_INDCON_BYTE_PARAM, i * PROFILE_SIZE + 1);
			if (subscriberInterfaceParam->value().toUInt() == interfaceIdx && subscriberDeviceParam->value().toUInt() == deviceIdx)
				return i;
		}

	return -1;
}

DcParamCfg_v2 * ConnectionDiscretWidget::getParam(uint32_t addr, uint32_t idx, bool showError) const
{
	DcParamCfg_v2 *p = dynamic_cast<DcParamCfg_v2*>(m_controller->params_cfg()->get(addr, idx));
	if (!p && showError) {
		MsgBox::error(QString("Не удалось прочитать параметр(%1:%2): параметр не найден в шаблоне устройства")
			.arg(QString("0x%1").arg(QString("%1").arg(addr, 4, 16, QChar('0')).toUpper()))
			.arg(idx));
		return nullptr;
	}

	return p;
}