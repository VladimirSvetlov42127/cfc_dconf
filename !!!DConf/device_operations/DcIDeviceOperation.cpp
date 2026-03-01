#include "DcIDeviceOperation.h"

#include <qapplication.h>
#include <qdebug.h>

#include <dpc/sybus/channel/Channel.h>
#include <data_model/dc_data_manager.h>

using namespace Dpc::Sybus;

DcIDeviceOperation::DcIDeviceOperation(const QString &name, DcController *device, int stepsCount, QObject *parent) :
	QObject(parent),
	m_name(name),
	m_device(device),
	m_state(NoErrorState),
	m_stepsCount(stepsCount),
	m_isCheckSoftIdEnabled(false)
{
	connect(channel().get(), &Channel::debug, this, &DcIDeviceOperation::onChannelDebug);
	connect(channel().get(), &Channel::info, this, &DcIDeviceOperation::onChannelInfo);
}

DcIDeviceOperation::State DcIDeviceOperation::state() const
{
	return m_state;
}

QStringList DcIDeviceOperation::reportList() const
{
	return m_reportsList;
}

void DcIDeviceOperation::start()
{
	addInfo(QString("Старт операции '%1'").arg(m_name));
	m_timer.start();

	if (!channel()) {
		abort("Не задано соединение");
		return;
	}	

	if (!before()) {
		abort();
		return;
	}

	if (!channel()->connect()) {
		abort(QString("Не удалось установить соединение c устройством: %1").arg(device()->channel()->errorMsg()));
		return;
	}

	if (!checkSoftId()) {
		abort();
		return;
	}

	if (!exec()) {
		abort();
		return;
	}

	if (!channel()->disconnect()) {
		abort(QString("Не удалось корректно разорвать соединение c устройством: %1").arg(device()->channel()->errorMsg()));
		return;
	}

	if (!after()) {
		abort();
		return;
	}

	setCurrentStep(m_stepsCount);
	finish(reportList().isEmpty() ? NoErrorState : WarningState);
}

void DcIDeviceOperation::addReport(const QString & report, bool warning)
{
	m_reportsList.append(report);
	if (warning)
		addWarning(report);
}

void DcIDeviceOperation::addInfo(const QString & msg)
{
	emit infoMsg(msg, device());
}

void DcIDeviceOperation::addWarning(const QString & msg)
{
	emit warningMsg(msg, device());
}

void DcIDeviceOperation::addError(const QString & msg)
{
	emit errorMsg(msg, device());
}

void DcIDeviceOperation::addDebug(const QString & msg, int level)
{
	emit debugMsg(msg, level, device());
}

void DcIDeviceOperation::emitProgress(int currentStepValue)
{
	auto totalProgressPerStep = 100.0 / m_stepsCount;
	int currentStepProgress = static_cast<double>(currentStepValue) / m_currentStepTotal * totalProgressPerStep;
	int currentStepOffset = static_cast<double>(m_currentStep) / m_stepsCount * 100;
	auto value = currentStepOffset + currentStepProgress;
	emit progress(value);
}

bool DcIDeviceOperation::saveConfig(DcBoard* board)
{
	auto boardId = board ? board->slot() : -1;
	if (!channel()->saveConfig(boardId)) {
		addError(QString("Не удалось сохранить конфигурацию: %1").arg(channel()->errorMsg()));
		return false;
	}

	return true;
}

bool DcIDeviceOperation::restart(DcBoard* board)
{
	auto boardId = board ? board->slot() : -1;
	if (!channel()->restartDevice(Channel::HardReset, boardId)) {
		addError(QString("Не удалось выполнить перезагрузку: %1").arg(channel()->errorMsg()));
		return false;
	}

	return true;
}

DcController * DcIDeviceOperation::device() const
{
	return m_device;
}

ChannelPtr DcIDeviceOperation::channel() const
{
	return device()->channel();
}

void DcIDeviceOperation::setCheckSoftIdEnabled(bool enabled)
{
	m_isCheckSoftIdEnabled = enabled;
}

void DcIDeviceOperation::abort(const QString & msg)
{
	if (!msg.isEmpty())
		addError(msg);

	finish(ErrorState);
}

void DcIDeviceOperation::setCurrentStep(int step, int total)
{
	m_currentStep = step;
	m_currentStepTotal = total;
	emitProgress(0);
}

QString DcIDeviceOperation::configArchiveDevicePath()
{
	return QString("0:/conf.zip");
}

void DcIDeviceOperation::onChannelDebug(int level, const QString &msg)
{
	addDebug(msg, level);
}

void DcIDeviceOperation::onChannelInfo(const QString& msg)
{
	addInfo(msg);
}

void DcIDeviceOperation::finish(DcIDeviceOperation::State state)
{
	if (channel()->thread() != qApp->thread())
		channel()->moveToThread(nullptr);

	m_state = state;
	QString msg = QString("Операция '%1' завершилась %3, время: %2 секунды").arg(m_name).arg(m_timer.elapsed() / 1000.0);
	switch (state)
	{
	case DcIDeviceOperation::NoErrorState: addInfo(msg.arg("успешно"));	break;
	case DcIDeviceOperation::WarningState: addWarning(msg.arg("с предупреждениями")); break;
	case DcIDeviceOperation::ErrorState: addError(msg.arg("неудачно"));	break;
	default: return;
	}

	emit finished(state);
}

bool DcIDeviceOperation::checkSoftId()
{
	if (!m_isCheckSoftIdEnabled)
		return true;

	addInfo("Проверка совместимости шаблона конфигурации и устройства...");

	auto p = channel()->param(SP_SOFTWARE_ID);
	if (!p) {
		addError(QString("Не удалось прочитать код ПО с устойства: %1").arg(channel()->errorMsg()));
		return false;
	}
	uint deviceSoftId = p->value<uint16_t>();

	auto param = device()->params_cfg()->get(SP_SOFTWARE_ID, 0);
	if (!param) {
		addError(QString("Не удалось прочитать код ПО из шаблона конфигурации: %1").arg(channel()->errorMsg()));
		return false;
	}
	auto localSoftId = param->value().toUInt();

	addInfo(QString("Коды ПО: на устройстве %1, в шаблоне конфигурации %2").arg(deviceSoftId).arg(localSoftId));
	if (!gDataManager.isCompatibleDevices(localSoftId, deviceSoftId)) {
		addError("Не совместимые коды ПО");
		return false;
	}

	return true;
}
