#pragma once
#include "IMatrixDelegate.h"

class AlgMatrixDelegate : public IMatrixDelegate
{
public:
	AlgMatrixDelegate(std::map<QStandardItem*, BindMatrixElem*> *mmap, DcController *controller, QObject *parent = 0);
	virtual ~AlgMatrixDelegate() = default;

protected:
	virtual QString getBusySignalName(BindMatrixElem* element) const;
	virtual bool isSignalBusy(BindMatrixElem* element) const;
	virtual bool removeElementBind(BindMatrixElem* element);
	virtual IBindDialog* createDialog(BindMatrixElem* element);
};

