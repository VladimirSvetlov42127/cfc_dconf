#include "OutputBindDialog.h"

#include <dpc/gui/dialogs/msg_box/MsgBox.h>
#include <gui/forms/table_of_bindings/dialogs/models/OutputBindDlgModel.h>
#include <gui/forms/table_of_bindings/dialogs/signal_names_functions.h>

using namespace Dpc::Gui;

OutputBindDialog::OutputBindDialog(DcController *controller, DcSignal * signal, QWidget * parent)
	: IBindDialog(controller, parent),
	m_PhysicOutput(signal)
{
	setModel(new OutputBindDlgModel(controller, ui.projTree, parent));
	m_model->fillTree();
	bool ok = connect(m_model, &IDlgModel::signal_newSignalChoosed, this, &IBindDialog::slot_newSignalChoosed);
	Q_ASSERT(ok);
}

OutputBindDialog::~OutputBindDialog()
{
}

void OutputBindDialog::selectSignal()
{
	if (!m_controller)
		return;

	QModelIndex index = ui.projTree->currentIndex();
	if (!index.isValid()) {
		return;
	}

	DcSignal* sourseSignal = m_model->getItemData(index);	// Получаем сигнал для привязки к выходу, он - sourse
	if (sourseSignal == nullptr) {
		return;
	}
	if (!ui.editSignalName->text().isEmpty() && sourseSignal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
		sourseSignal->updateName(ui.editSignalName->text());
	}

	DcMatrixElementSignal *bind = new DcMatrixElementSignal(m_controller->index(), sourseSignal->index(), m_PhysicOutput->index(), "");
	if (m_controller->matrix_signals()->add(bind, false))
		this->done(QDialog::Accepted);
	else {
		MsgBox::error("Невозможно добавить выбранную связь");
		this->done(QDialog::Rejected);
	}
}

void OutputBindDialog::newSignalChoosed(DcSignal* signal)
{
	if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
		if (signal->name().contains(VIRTUAL_DEFAULT_NAME))
			ui.editSignalName->setText(m_PhysicOutput->name());
		else {
			QString name = addOutputTag(m_PhysicOutput->name(), signal->name());
			ui.editSignalName->setText(name);
		}
	}
	else {
		ui.editSignalName->setText(signal->name());
	}
}
