#pragma once

#include <qwidget.h>

#include <data_model/storage_for_controller_params/dc_param.h>

class ParamEditor : public QWidget
{
	Q_OBJECT
public:
	ParamEditor(QWidget *parent = nullptr);
	ParamEditor(DcParam *param, QWidget *parent = nullptr);
	virtual ~ParamEditor();
	
	DcParam* param() const;
	virtual void setReadOnly(bool readOnly);
	virtual void setValue(const QString &value) = 0;

signals:
	void valueChanged(const QString &newValue, const QString &oldValue);

protected:
	void setParamValue(const QString &value);

private:
	DcParam *m_param;
	QString m_value;
};