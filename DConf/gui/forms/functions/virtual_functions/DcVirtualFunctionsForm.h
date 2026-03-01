#pragma once

#include <gui/forms/DcFormFactory.h>
#include <QTableWidget>

class DcVirtualFunctionsForm : public DcForm
{
	Q_OBJECT

public:
	DcVirtualFunctionsForm(DcController *controller, const QString &path);

	static bool isAvailableFor(DcController *controller);
	static void fillReport(DcIConfigReport *report);
	
//private slots:
//	void onFunctionEditorChange(QString value);

private:
	QTableWidget* Table() { return _table; }

	//	Элементы формы
	QTableWidget* _table;
};

REGISTER_FORM(DcVirtualFunctionsForm, DcMenu::virtual_functions);
