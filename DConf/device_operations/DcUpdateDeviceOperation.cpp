#include "DcUpdateDeviceOperation.h"
#include <dpc/sybus/channel/Channel.h>

#include <qfiledialog.h>


#ifdef _THRADED_CHANNEL
using namespace Dc::SyBUS;
#else
using namespace Dpc::Sybus;
#endif // _THRADED_CHANNEL

namespace {
	enum OperationSteps {
		UpdateStep = 0,

		TotalStepsCount
	};
}

    DcUpdateDeviceOperation::DcUpdateDeviceOperation(DcController * device, const QString &fileName, QObject * parent) :
	DcIDeviceOperation("Обновление устройства", device, TotalStepsCount, parent), 
	m_fileName(fileName)
{
    connect(channel().get(), &Channel::error, this, [=]() { addError(channel()->errorMsg()); });
    connect(channel().get(), &Channel::progress, this, &DcUpdateDeviceOperation::emitProgress);
}

//bool DcUpdateDeviceOperation::before()
//{
//	m_fileName = QFileDialog::getOpenFileName(0, "Выбрать файлы", "", "*.img");
//	if (m_fileName.isEmpty()) {
//		addWarning("Файл обновления не был выбран.");
//		return false;
//	}
//
//	return true;
//}

bool DcUpdateDeviceOperation::exec()
{
	setCurrentStep(UpdateStep);
    if (!channel() || !channel()->updateDevice(m_fileName)) {
		addError(QString("Не удалось обновить устройство."));
		return false;
	}

	return true;
}
