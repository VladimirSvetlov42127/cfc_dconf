#pragma once
#include "IBindDialog.h"

class InputBindDialog :	public IBindDialog
{
public:
	InputBindDialog(DcController *controller, DcSignal* signal, QWidget *parent = Q_NULLPTR);
	virtual ~InputBindDialog();

private:
	DcSignal *m_physicInput;

protected:
	virtual void selectSignal();
	virtual void newSignalChoosed(DcSignal* signal);
};

