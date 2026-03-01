#include "DateTimeWidget.h"

#include <qdatetimeedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtimezone.h>

#include <dpc/gui/dialogs/msg_box/MsgBox.h>
#include <dpc/gui/gui.h>

#include <device_operations/DcInfoOperation.h>
#include <device_operations/DcTimeSetOperation.h>

#include <journal/JSource.h>
#include <gui/editors/EditorsManager.h>

using namespace Dpc::Sybus;
using namespace Dpc::Gui;

namespace {
	QString toString(const QVariant &offset)
	{
		if (offset.isNull())
			return QString();

		auto value = offset.toInt();
		return value > 0 ? QString("+%1").arg(value) : QString::number(value); 
	}
}

DateTimeWidget::DateTimeWidget(DcController *controller, QWidget *parent) :
	IDeviceTabWidget(controller, parent),
	m_dateEdit(new QDateEdit),
	m_timeEdit(new QTimeEdit),
	m_offsetComboBox(new QComboBox)
{
	m_dateEdit->setCalendarPopup(true);
	m_dateEdit->setDisplayFormat("dd.MM.yyyy");
	m_dateEdit->setDate(QDate());

	m_timeEdit->setTime(QTime());
	m_timeEdit->setCalendarPopup(true);
	m_timeEdit->setDisplayFormat("hh:mm:ss");

	for (int i = -12; i <= 14; i++)
		m_offsetComboBox->addItem(::toString(i), i);
	m_offsetComboBox->setCurrentIndex(-1);

	auto setDateTimeFunc = [=]() {
	};

	auto setDTButton = new QPushButton("Установить заданное время");
	connect(setDTButton, &QPushButton::clicked, this, [=]() {
		setDateTime(QDateTime(m_dateEdit->date(), m_timeEdit->time(), Qt::UTC), m_offsetComboBox->currentData());
	});

	auto setSysDTButton = new QPushButton("Установить системное время");
	connect(setSysDTButton, &QPushButton::clicked, this, [=]() {
		auto dt = QDateTime::currentDateTime();
		setDateTime(QDateTime(dt.date(), dt.time(), Qt::UTC), dt.offsetFromUtc() / 3600);
	});

	EditorsManager em(controller, new QGridLayout(this));
	em.addWidget(m_dateEdit, "Дата");
	em.addWidget(m_timeEdit, "Время");
	em.addWidget(m_offsetComboBox, "Часовой пояс");
	em.addWidget(setDTButton, 0, 1, 2);
	em.addWidget(setSysDTButton, 0, 1, 2);

	em.addStretch();
}

DateTimeWidget::~DateTimeWidget()
{
}

void DateTimeWidget::deviceOperationFinished(DcIDeviceOperation* op)
{
	if (auto o = dynamic_cast<DcInfoOperation*>(op); o) {
		if (DcIDeviceOperation::ErrorState == o->state())
			return;

		auto dt = o->dateTime();
		if (dt.isValid()) {
			m_dateEdit->setDate(dt.date());
			m_timeEdit->setTime(dt.time());
		}

		auto offset = o->offset();
		m_offsetComboBox->setEnabled(offset.isValid());
		m_offsetComboBox->setCurrentIndex(-1);
		m_offsetComboBox->setToolTip(QString());
		m_offsetComboBox->setStyleSheet(QString());
		if (!offset.isValid()) {
			m_offsetComboBox->setToolTip("Не поддерживается протоколом синхронизации");
			return;
		}
		
		for (size_t i = 0; i < m_offsetComboBox->count(); i++)
			if (m_offsetComboBox->itemData(i).toInt() == offset.toInt()) {
				m_offsetComboBox->setCurrentIndex(i);
				break;
			}

		auto configOffset = device()->getTimeOffset();
		if (configOffset != offset) {

			auto toolTip = QString("Часовой пояс в конфигурации(%1) отличается от установленого в устройстве(%2)");
			m_offsetComboBox->setToolTip(toolTip.arg(::toString(configOffset), ::toString(offset)));
			m_offsetComboBox->setStyleSheet(QString("QComboBox { background-color: %1; }").arg(QColor(Qt::darkYellow).name()));
		}
	}
}

void DateTimeWidget::setDateTime(const QDateTime& dt, const QVariant& offset)
{	
	emit newDeviceOperation(new DcTimeSetOperation(device(), dt, offset), false, false);
}
