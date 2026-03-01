#include "DcFormatDriveOperation.h"

#include <dpc/sybus/channel/Channel.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		FormatStep = 0,

		TotalStepsCount
	};
}

DcFormatDriveOperation::DcFormatDriveOperation(DcController * device, int drive, QObject * parent) :
	DcIDeviceOperation("Форматирование тома", device, TotalStepsCount, parent),
	m_drive(drive)
{
	connect(channel().get(), &Channel::error, this, [=]() { addError(channel()->errorMsg()); });
}

bool DcFormatDriveOperation::exec()
{
	if (!channel()->formatDrive(m_drive)) {
		addError(QString("Не удалось отформатировать том %1").arg(m_drive));
		return false;
	}

	return true;
}