#pragma once
#include "IBindDialog.h"

class OutputBindDialog :	public IBindDialog
{
public:
	OutputBindDialog(DcController *controller, DcSignal* signal, QWidget *parent = Q_NULLPTR);
	virtual ~OutputBindDialog();

private:
	DcSignal *m_PhysicOutput;

protected:
	virtual void selectSignal();
	virtual void newSignalChoosed(DcSignal* signal);
};


