#pragma once

#include <device_operations/DcIDeviceOperation.h>

class DcResetOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
	DcResetOperation(DcController *device, QObject *parent = nullptr);

	QString ipAddress() const;

protected:
	virtual bool exec() override;

private:
	bool clearFiles();

private:
	QString m_ip;
};