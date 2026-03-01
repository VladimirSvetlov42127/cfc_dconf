#pragma once

#include <device_operations/DcIDeviceOperation.h>

class QTemporaryDir;

class DcTemporaryConfigOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
	DcTemporaryConfigOperation(const QString &name, DcController *device, int stepsCount = 1, QObject *parent = nullptr);
	~DcTemporaryConfigOperation();

protected:
	virtual bool before() override;
	virtual bool exec() override;

	DcController* tempDevice() const { return m_tempDevice; }

private:
	bool readConfig();

private:
	QTemporaryDir *m_tempDir;
	DcController *m_tempDevice;
};