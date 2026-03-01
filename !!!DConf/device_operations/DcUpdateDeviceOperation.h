#pragma once

#include <device_operations/DcIDeviceOperation.h>

class DcUpdateDeviceOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
    DcUpdateDeviceOperation(DcController *device,const QString& fileName, QObject *parent = nullptr);

protected:
	//Выбор файла вынесен наружу
	//virtual bool before() override;
	virtual bool exec() override;

private:
    const QString m_fileName;
};
