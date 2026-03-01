#pragma once

#include <qdatetime.h>

#include <device_operations/DcIDeviceOperation.h>
#include <dpc/sybus/ParamPack.h>

class DcInfoOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
	struct ParamInfo
	{
		Dpc::Sybus::ParamPackPtr values;
		QStringList names;
	};
	using ParamInfoContainer = std::map<int, ParamInfo>;
	DcInfoOperation(DcController *device, QObject *parent = nullptr);

	ParamInfoContainer params() const;
	QDateTime dateTime() const;
	QVariant offset() const { return m_offset; }

protected:
	virtual bool exec() override;

private:
	bool readParams();

private:
	ParamInfoContainer m_params;
	QDateTime m_dateTime;
	QVariant m_offset;
};