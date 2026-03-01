#pragma once

#include <gui/forms/DcFormFactory.h>

class DcBoardsForm : public DcForm
{

public:
	DcBoardsForm(DcController *controller, const QString &path);

	static bool isAvailableFor(DcController *controller);
	static void fillReport(DcIConfigReport *report);
};

REGISTER_FORM(DcBoardsForm, DcMenu::boards);