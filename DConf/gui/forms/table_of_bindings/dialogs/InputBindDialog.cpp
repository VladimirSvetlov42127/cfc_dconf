#include "InputBindDialog.h"

#include <dpc/gui/dialogs/msg_box/MsgBox.h>
#include <gui/forms/table_of_bindings/dialogs/models/InputBindDlgModel.h>
#include <gui/forms/table_of_bindings/dialogs/signal_names_functions.h>

using namespace Dpc::Gui;

InputBindDialog::InputBindDialog(DcController *controller, DcSignal * signal, QWidget * parent)
	: IBindDialog(controller, parent),
	m_physicInput(signal)
{
	setModel(new InputBindDlgModel(controller, ui.projTree, parent));
	m_model->fillTree(); 
	bool ok = connect(m_model, &IDlgModel::signal_newSignalChoosed, this, &IBindDialog::slot_newSignalChoosed);
	Q_ASSERT(ok);
}

InputBindDialog::~InputBindDialog()
{
}

void InputBindDialog::selectSignal()
{
	if (!m_controller)
		return;

	QModelIndex index = ui.projTree->currentIndex();
	if (!index.isValid()) {
		return;
	}

	DcSignal* physicalOutput = m_model->getItemData(index);	// Получаем выбранный сигнал, только физический
	if (physicalOutput == nullptr) {
		return;
	}

	DcMatrixElementSignal *bind = new DcMatrixElementSignal(m_controller->index(), m_physicInput->index(), physicalOutput->index(), "");
	if (m_controller->matrix_signals()->add(bind, false))
		this->done(QDialog::Accepted);
	else {
		MsgBox::error("Невозможно добавить выбранную связь");
		this->done(QDialog::Rejected);
	}
		
}

void InputBindDialog::newSignalChoosed(DcSignal* signal)
{
	if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
		if(signal->name().contains(VIRTUAL_DEFAULT_NAME))
			ui.editSignalName->setText(m_physicInput->name());
		else
			ui.editSignalName->setText(addInputTag(m_physicInput->name(), signal->name()));
	}
	else {
		ui.editSignalName->setText(m_physicInput->name());
	}
		
}
