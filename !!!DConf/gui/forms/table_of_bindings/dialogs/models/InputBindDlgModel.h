#pragma once

#include <gui/forms/table_of_bindings/dialogs/models/IDlgModel.h>

class InputBindDlgModel : public IDlgModel
{
	Q_OBJECT

public:
	InputBindDlgModel(QObject *parent = nullptr);
	InputBindDlgModel(DcController *controller,  QTreeView *ptree, QObject *parent = nullptr);
	virtual ~InputBindDlgModel();
		
	virtual void fillTree();

};
