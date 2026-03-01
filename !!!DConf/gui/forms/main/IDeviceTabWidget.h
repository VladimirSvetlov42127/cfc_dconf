#pragma once

#include <qwidget.h>

#include <data_model/dc_controller.h>

class DcIDeviceOperation;

class IDeviceTabWidget : public QWidget
{
	Q_OBJECT

public:
	IDeviceTabWidget(DcController *controller, QWidget *parent = nullptr);
	virtual ~IDeviceTabWidget();

	virtual void deviceOperationFinished(DcIDeviceOperation* op);

signals:
	void newDeviceOperation(DcIDeviceOperation*, bool, bool);

protected:
	DcController* device() const;

private:
	DcController *m_device;
};