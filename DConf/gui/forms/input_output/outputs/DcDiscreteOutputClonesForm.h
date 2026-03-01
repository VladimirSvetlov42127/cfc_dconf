#pragma once

#include <gui/forms/DcFormFactory.h>

class DcDiscreteOutputClonesForm : public DcForm
{
	Q_OBJECT
public:
	DcDiscreteOutputClonesForm(DcController *controller, const QString &path);

	static bool isAvailableFor(DcController *controller);
	static void fillReport(DcIConfigReport *report);

};

REGISTER_FORM(DcDiscreteOutputClonesForm, DcMenu::output_clones);
