#include "DcTimeSetOperation.h"

#include <dpc/sybus/channel/Channel.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		FormatStep = 0,

		TotalStepsCount
	};
}

DcTimeSetOperation::DcTimeSetOperation(DcController* device, const QDateTime& dateTime, const QVariant& offset, QObject * parent) :
	DcIDeviceOperation("Установка даты и времени", device, TotalStepsCount, parent),
	m_dateTime(dateTime), 
	m_offset(offset)
{
	connect(channel().get(), &Channel::error, this, [=]() { addError(channel()->errorMsg()); });
}

bool DcTimeSetOperation::exec()
{
	auto currentOffset = channel()->dateTimeOffset();
	if (Channel::NoError != channel()->errorCode())
		return false;

	if (currentOffset.isValid()) {
		m_dateTime = m_dateTime.addSecs(-1 * (m_offset.toInt() * 3600));

		if (m_offset.isValid())
			m_dateTime = m_dateTime.addSecs(currentOffset.toInt() * 3600);
	}

	if (!channel()->setDateTime(m_dateTime))
		return false;

	if (m_offset.isValid() && !channel()->setDateTimeOffset(m_offset.value<int8_t>()))
		return false;

	if (!saveConfig())
		return false;

	if (!restart())
		return false;

	return true;
}