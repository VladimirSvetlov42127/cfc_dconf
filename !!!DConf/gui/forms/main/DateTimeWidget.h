#pragma once

#include <qwidget.h>

#include <gui/forms/main/IDeviceTabWidget.h>

class QDateEdit;
class QTimeEdit;
class QComboBox;

class DateTimeWidget : public IDeviceTabWidget
{
	Q_OBJECT

public:
	DateTimeWidget(DcController *controller, QWidget *parent = nullptr);
	~DateTimeWidget();

	virtual void deviceOperationFinished(DcIDeviceOperation* op) override;

private:
	void setDateTime(const QDateTime& dt, const QVariant &offset);

private:
	QDateEdit* m_dateEdit;
	QTimeEdit* m_timeEdit;
	QComboBox *m_offsetComboBox;
};