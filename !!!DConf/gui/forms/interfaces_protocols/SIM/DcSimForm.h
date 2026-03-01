#pragma once

#include <gui/forms/DcFormFactory.h>

class DcSimForm : public DcForm
{

public:
	DcSimForm(DcController *controller, const QString &path);

	static bool isAvailableFor(DcController *controller);
	static void fillReport(DcIConfigReport *report);
};

REGISTER_FORM(DcSimForm, DcMenu::sim);
