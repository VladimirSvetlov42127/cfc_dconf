#pragma once

#include <qwidget.h>
#include <qboxlayout.h>
#include <qdebug.h>


#include <data_model/dc_controller.h>

class DcForm;
class QLabel;

using DcFormPtr = std::shared_ptr<DcForm>;

class DcForm : public QWidget 
{
public:
	explicit DcForm(DcController *controller, const QString &path, const QString &title = QString(), bool hasShape = true);
	virtual ~DcForm();

	DcController* controller() const;
	virtual void activate() {}

protected:
	QWidget* centralWidget() const;
	void setTitle(const QString &title);

	struct Param {
		int32_t addr;
		int32_t index = 0; };

	static bool hasAny(DcController *controller, const QList<Param> &params);
	static bool hasAll(DcController *controller, const QList<Param> &params);

private:
	DcController *m_controller;

	QLabel *m_titleLabel;
	QWidget *m_centralWidget;
};

Q_DECLARE_METATYPE(DcFormPtr)
