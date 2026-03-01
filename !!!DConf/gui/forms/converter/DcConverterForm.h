#pragma once

#include <gui/forms/DcFormFactory.h>

class DcConverterForm : public DcForm
{
	Q_OBJECT
public:
	DcConverterForm(DcController *controller, const QString &path);

	static bool isAvailableFor(DcController *controller);
	static void fillReport(DcIConfigReport *report);

private:

};

REGISTER_FORM(DcConverterForm, DcMenu::converter);

