#include "AlgBindDialog.h"

#include <dpc/gui/dialogs/msg_box/MsgBox.h>
#include <gui/forms/DcSignalManager.h>
#include <gui/forms/table_of_bindings/dialogs/models/AlgBindDlgModel.h>
#include <gui/forms/table_of_bindings/dialogs/signal_names_functions.h>

using namespace Dpc::Gui;

AlgBindDialog::AlgBindDialog(DcController *controller, DcAlgIO * alg, QWidget * parent)
	: IBindDialog(controller), m_alg(alg)
{
	setModel(new AlgBindDlgModel(controller, alg, ui.projTree, parent));
	m_model->fillTree();
	bool ok = connect(m_model, &IDlgModel::signal_newSignalChoosed, this, &IBindDialog::slot_newSignalChoosed);
	Q_ASSERT(ok);
}

void AlgBindDialog::selectSignal()
{
	if (!m_controller)
		return;

	QModelIndex index = ui.projTree->currentIndex();	// Индекс выбранного элемента в модели
	if (!index.isValid()) {
		return;
	}

	DcSignal* signal = m_model->getItemData(index);	// Выбранный сигнал из списка
	if (signal == nullptr) {
		return;
	}

	if (m_controller->matrix_alg()->exist(m_alg->index(), signal->index()))	// Такая связь уже есть
		return;

	QString name = ui.editSignalName->text();	// Пользовательское имя
	
	if (m_alg->direction() == DEF_SIG_DIRECTION_OUTPUT) {

		if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
            DcMatrixElementAlg *palg = new DcMatrixElementAlg(m_controller->index(), m_alg->index(), signal->index(), "");

			if (!name.isEmpty()) {
				signal->updateName(name);
			}

			m_controller->matrix_alg()->add(palg, false);
		}
		else {
			// Ищем свободный виртуальный вход
            DcSignalManager signalManager(m_controller);
            DcSignal *pvdin = signalManager.getFreeVDin(); 	// Возвращает последний незанятый виртуальный выход
			if (pvdin == nullptr) {
				MsgBox::error("Не найдено свободных виртуальных входов");
				return;
			}
			pvdin->updateName(name);	// Обновляем имя выхода

            DcMatrixElementAlg *palg = new DcMatrixElementAlg(m_controller->index(), m_alg->index(), pvdin->index(), "");	// Добавляем связь с виртуальным входом
			m_controller->matrix_alg()->add(palg, false);

			DcMatrixElementSignal *poutmatrix = new DcMatrixElementSignal(m_controller->index(), pvdin->index(), signal->index(), "");
			m_controller->matrix_signals()->add(poutmatrix, false);
		}

	}
	else if (m_alg->direction() == DEF_SIG_DIRECTION_INPUT) {

        DcMatrixElementAlg *palg = new DcMatrixElementAlg(m_controller->index(), m_alg->index(), signal->index(), "");

		if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {	// Обновляем имя
			if (!name.isEmpty()) {
				signal->updateName(name);
			}
		}

		m_controller->matrix_alg()->add(palg, false);
	}

	this->done(QDialog::Accepted);
}

void AlgBindDialog::newSignalChoosed(DcSignal * signal)
{
	QString choosenSignalName = signal->name();
	if (choosenSignalName[0] == ' ')	// Если имя начинается с пробела, удаляем его
		choosenSignalName = choosenSignalName.remove(0, 1);

	if (m_alg->direction() == IO_DIRECTION_OUTPUT) {	// Если выбираем привязку для выхода алгоритма
		if (signal->subtype() == DefSignalSubType::DEF_SIG_SUBTYPE_VIRTUAL) {	// Если выбираем виртуальный вход
			ui.editSignalName->setText(m_alg->name());	// То вирт вход переименовываем в имя алгоритма
		}
		else
			ui.editSignalName->setText(addOutputTag(choosenSignalName, m_alg->name())); // Если не виртуальный, добавляем тег, куда этот выход алгоритма приходит, ставим справа
		ui.editSignalName->setEnabled(true);
	}
	else if (m_alg->direction() == IO_DIRECTION_INPUT) {	// Если привязываем ко входу алгоритма	
		if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
			ui.editSignalName->setText(addInputTag(choosenSignalName, m_alg->name()));	// То добавляем тег в начало имени
			ui.editSignalName->setEnabled(true);
		}
		else {
			ui.editSignalName->setText(choosenSignalName);	// То добавляем тег в начало имени
			ui.editSignalName->setEnabled(false);
		}
	}
}
