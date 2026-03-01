#include "DcTemporaryConfigOperation.h"

#include <qtemporarydir.h>
#include <qdebug.h>

#include <data_model/storage_for_controller_params/DcController_v2.h>
#include <dpc/sybus/channel/Channel.h>
#include <device_operations/DcReadConfigOperation.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		ReadConfigStep = 0,

		TotalStepsCount
	};
}

DcTemporaryConfigOperation::DcTemporaryConfigOperation(const QString &name, DcController *device, int stepsCount, QObject * parent) :
	DcIDeviceOperation(name, device, stepsCount, parent),
	m_tempDir(new QTemporaryDir),
	m_tempDevice(nullptr)
{
	setCheckSoftIdEnabled(true);
}

DcTemporaryConfigOperation::~DcTemporaryConfigOperation()
{
	delete m_tempDevice;
	delete m_tempDir;	
}

bool DcTemporaryConfigOperation::before()
{
	if (!m_tempDir->isValid()) {
		addError(QString("Не удалось создать временную папку для работы с файлами: %1").arg(m_tempDir->errorString()));
		return false;
	}

	m_tempDevice = dynamic_cast<DcController_v2*>(device())->clone(11111, m_tempDir->filePath("uconf.db"), device()->name());
	if (!m_tempDevice) {
		addError("Не удалось склонировать текущий шаблон конфигурации устройства");
		return false;
	}

	return true;
}

bool DcTemporaryConfigOperation::exec()
{
	if (!readConfig())
		return false;

	return true;
}

bool DcTemporaryConfigOperation::readConfig()
{
	addInfo("Вычитывание конфигурации во временную папку...");

	auto readOperation = std::make_shared<DcReadConfigOperation>(m_tempDevice, this);
	connect(readOperation.get(), &DcIDeviceOperation::warningMsg, this, &DcTemporaryConfigOperation::addWarning);
	connect(readOperation.get(), &DcIDeviceOperation::errorMsg, this, &DcTemporaryConfigOperation::addError);
	connect(readOperation.get(), &DcIDeviceOperation::progress, this, &DcTemporaryConfigOperation::emitProgress);

	setCurrentStep(ReadConfigStep);
	readOperation->start();
	if (ErrorState == readOperation->state()) {
		return false;
	}

	if (WarningState == readOperation->state()) {
		for (auto &it : readOperation->reportList())
			addReport(it, false);

		if (readOperation->failedFiles().size()) {
			addError(QString("Не удалось вычитать все конфигурационные файлы с устройства"));
			return false;
		}

		for (auto &it : readOperation->failedParams()) {
			auto param = it.first;
			auto board = it.second;
			if (board)
				board->params()->remove(param->addr(), param->param());
			else
				m_tempDevice->params_cfg()->remove(param->addr(), param->param());
		}

	}

	return true;
}