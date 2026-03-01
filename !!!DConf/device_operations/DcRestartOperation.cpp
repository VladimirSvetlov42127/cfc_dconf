#include "DcRestartOperation.h"

#include <dpc/sybus/channel/Channel.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		RestartStep = 0,

		TotalStepsCount
	};
}

DcRestartOperation::DcRestartOperation(DcController * device, int mode, QObject * parent) :
	DcIDeviceOperation("Перезагрузка устройства", device, TotalStepsCount, parent),
	m_mode(mode)
{
}

bool DcRestartOperation::exec()
{
	if (!channel()->restartDevice((Channel::ResetMode) m_mode)) {
		QString msg("Не удалось перезагрузить устройство");
		if (m_mode == Channel::MinimalModeReset)
			msg.append(" в минимальный режим");
		addError(QString("%1: %2").arg(msg).arg(channel()->errorMsg()));
		return false;
	}

	return true;
}