#pragma once

#include <gui/forms/table_of_bindings/dialogs/models/IDlgModel.h>

class OutputBindDlgModel :	public IDlgModel
{
	Q_OBJECT

public:
	OutputBindDlgModel(QObject *parent = nullptr);
	OutputBindDlgModel(DcController *controller, QTreeView *ptree, QObject *parent = nullptr);
	virtual ~OutputBindDlgModel() = default;

	virtual void fillTree();

};

