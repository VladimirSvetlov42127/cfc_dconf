#pragma once

#include "IBindDialog.h"

class AlgBindDialog : public IBindDialog
{
public:
	AlgBindDialog(DcController *controller, DcAlgIO* alg, QWidget *parent = Q_NULLPTR);
	virtual ~AlgBindDialog() = default;

private:
	DcAlgIO *m_alg;

protected:
	virtual void selectSignal();
	virtual void newSignalChoosed(DcSignal* signal);
};

