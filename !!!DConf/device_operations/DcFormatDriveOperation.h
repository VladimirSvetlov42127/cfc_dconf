#pragma once

#include <device_operations/DcIDeviceOperation.h>

class DcFormatDriveOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
	DcFormatDriveOperation(DcController *device, int drive, QObject *parent = nullptr);

protected:
	virtual bool exec() override;

private:
	int m_drive;
};