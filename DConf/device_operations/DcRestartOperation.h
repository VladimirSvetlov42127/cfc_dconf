#pragma once

#include <device_operations/DcIDeviceOperation.h>

class DcRestartOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
	DcRestartOperation(DcController *device, int mode, QObject *parent = nullptr);

protected:
	virtual bool exec() override;

private:
	int m_mode;
};