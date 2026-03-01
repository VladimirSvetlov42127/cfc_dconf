#pragma once

#include <qwidget.h>

class DcController;
class DcParamCfg_v2;
class QComboBox;

class ConnectionDiscretWidget : public QWidget
{
	Q_OBJECT

public:
	ConnectionDiscretWidget(DcController *controller, int interfaceIdx, int deviceIdx, const QString &deviceName, QWidget *parent = nullptr);
	~ConnectionDiscretWidget();

	void updateList();

private slots:
	void onComboBoxIndexChanged(int idx);

private:
	int subscriberProfile(int interfaceIdx, int deviceIdx) const;
	DcParamCfg_v2 * getParam(uint32_t addr, uint32_t idx, bool showError = true) const;

private:
	DcController *m_controller;
	int m_interfaceIdx;
	int m_deviceIdx;	
	int m_previousComboBoxIdx;
	QString m_deviceName;

	QComboBox *m_comboBox;
};