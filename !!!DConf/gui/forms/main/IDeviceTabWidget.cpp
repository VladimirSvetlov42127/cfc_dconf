#include "IDeviceTabWidget.h"

#include <qdatetimeedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <journal/JSource.h>

#include <gui/editors/EditorsManager.h>
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

using namespace Dpc::Sybus;
using namespace Dpc::Gui;

namespace {
}

IDeviceTabWidget::IDeviceTabWidget(DcController *controller, QWidget *parent) :
	QWidget(parent),
	m_device(controller)
{
}

IDeviceTabWidget::~IDeviceTabWidget()
{
}

void IDeviceTabWidget::deviceOperationFinished(DcIDeviceOperation* op)
{
}

DcController * IDeviceTabWidget::device() const
{
	return m_device;
}