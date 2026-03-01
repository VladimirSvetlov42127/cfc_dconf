#pragma once

#include <gui/forms/table_of_bindings/dialogs/models/IDlgModel.h>

class AlgBindDlgModel :	public IDlgModel
{
	Q_OBJECT

public:
	AlgBindDlgModel(DcController *controller, DcAlgIO* alg, QTreeView *ptree, QObject *parent = nullptr);
	virtual ~AlgBindDlgModel() = default;

	virtual void fillTree();

private:
	DcAlgIO *m_alg;
};

