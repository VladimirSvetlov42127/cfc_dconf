#pragma once

#include <qdatetime.h>

#include <device_operations/DcIDeviceOperation.h>

class DcTimeSetOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
	DcTimeSetOperation(DcController *device, const QDateTime &dateTime, const QVariant &offset, QObject *parent = nullptr);

protected:
	virtual bool exec() override;

private:
	QDateTime m_dateTime;
	QVariant m_offset;
};