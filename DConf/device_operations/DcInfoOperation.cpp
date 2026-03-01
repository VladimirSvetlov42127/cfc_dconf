#include "DcInfoOperation.h"

#include <dpc/sybus/channel/Channel.h>
#include <dpc/sybus/utils.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		ReadParamStep = 0,

		TotalStepsCount
	};
}

DcInfoOperation::DcInfoOperation(DcController * device, QObject * parent) :
	DcIDeviceOperation("Обновление информации", device, TotalStepsCount, parent)
{
}

DcInfoOperation::ParamInfoContainer DcInfoOperation::params() const
{
	return m_params;
}

QDateTime DcInfoOperation::dateTime() const
{
	return m_dateTime;
}

bool DcInfoOperation::exec()
{
	if (!readParams())
		return false;	

	return true;
}

bool DcInfoOperation::readParams()
{
	QHash<uint16_t, bool> neededParams; // addr, optional 

	auto requiredParams = DcController::specialParams(DcController::BaseParam);
	requiredParams.insert(SP_SM_ENTITY_STATUS);
	requiredParams.insert(SP_APP_ENTITY_STATUS);
	requiredParams.insert(SP_VOLUME_INFO);
	for (auto it : requiredParams) neededParams[it] = false;

	neededParams[SP_IP4_ADDR] = true;


	setCurrentStep(ReadParamStep, neededParams.size());
	int progress = 0;
	for (auto it = neededParams.begin(); it != neededParams.end() ; it++) {
		emitProgress(++progress);
		auto addr = it.key();
		auto isOptional = it.value();

		auto dim = channel()->dimension(addr);
		if (!dim) {
			if (Channel::DeviceErrorType == channel()->errorType() && isOptional) continue;
			if (neededParams[it.key()] == false) continue;
			addError(QString("Не удалось получить размерность параметра %1: %2").arg(toHex(addr)).arg(channel()->errorMsg()));
			return false; }

		auto dimSize = dim.subProfileCount();
		auto values = channel()->param(addr, 0, dimSize);
		if (Channel::NoError != channel()->errorType()) {
			addError(QString("Не удалось получить занчения параметра %1: %2").arg(toHex(addr)).arg(channel()->errorMsg()));
			return false;
		}

		auto namesPack = channel()->names(addr);
		if (Channel::NoError != channel()->errorType()) {
			addError(QString("Не удалось получить имена параметра %1: %2").arg(toHex(addr)).arg(channel()->errorMsg()));
			return false;
		}

		QStringList names;
		for (size_t i = 0; i < dimSize; i++) {
			QString name = namesPack->value<QString>();
			if (namesPack->count() == dimSize + 1)
				name = namesPack->value<QString>(i + 1);

			names << name;
		}

		m_params[addr] = { values, names };
	}

	m_dateTime = channel()->dateTime();
	if (!m_dateTime.isValid())
		return false;

	m_offset = channel()->dateTimeOffset();
    if (m_offset.isNull() && Channel::NoError != channel()->errorCode()) {
//		return false;
    }

	return true;
}
