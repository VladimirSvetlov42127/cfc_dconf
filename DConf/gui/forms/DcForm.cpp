#include "DcForm.h"

#include <qlabel.h>
#include <qscrollarea.h>
#include <qevent.h>

DcForm::DcForm(DcController *controller, const QString &path, const QString &title, bool hasShape) :
	m_controller(controller),
	m_titleLabel(new QLabel),
	m_centralWidget(new QWidget)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	QLabel *plablepath = new QLabel(path);
	plablepath->setStyleSheet("font-weight: regular; padding: 5px 0px 5px 0px;");
	QFont fontpath = plablepath->font();
	fontpath.setPixelSize(10);
	plablepath->setFont(fontpath);
	layout->addWidget(plablepath);

	m_titleLabel->setText(title);
	m_titleLabel->setStyleSheet("font-weight: bold; padding: 5px 0px 10px 0px;");
	QFont font = m_titleLabel->font();
	font.setPixelSize(14);
	m_titleLabel->setFont(font);
	layout->addWidget(m_titleLabel);

	QScrollArea *scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable(true);
	if (!hasShape) scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setWidget(m_centralWidget);
	layout->addWidget(scrollArea);
}

DcForm::~DcForm()
{
}

DcController* DcForm::controller() const
{
	return m_controller;
}

QWidget * DcForm::centralWidget() const
{
	return m_centralWidget;
}

void DcForm::setTitle(const QString & title)
{
	m_titleLabel->setText(title);
}

bool DcForm::hasAny(DcController * controller, const QList<Param>& params)
{
	for (auto &it : params) if (controller->getParam(it.addr, it.index)) return true;
	return false;
}

bool DcForm::hasAll(DcController * controller, const QList<Param>& params)
{
	for (auto &it : params)	if (!controller->getParam(it.addr, it.index)) return false;
	return true;
}
