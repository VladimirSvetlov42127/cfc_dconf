#include "OutputMatrixDelegate.h"

#include <gui/forms/table_of_bindings/dialogs/signal_names_functions.h>
#include <gui/forms/table_of_bindings/dialogs/OutputBindDialog.h>

using namespace Dpc::Gui;

OutputMatrixDelegate::OutputMatrixDelegate(std::map<QStandardItem*, BindMatrixElem*>* mmap, DcController *controller, QObject * parent)
	: IMatrixDelegate(mmap, controller, parent)
{
}

QString OutputMatrixDelegate::getBusySignalName(BindMatrixElem * element) const
{
	QString name;
	DcSignal *signalOutput = dynamic_cast<DcSignal *>(element);
	if (!signalOutput)
		return name;

	if(!signalOutput)
		return name;

	DcMatrixElementSignal* matrixelem = m_controller->matrix_signals()->getDst(signalOutput->index());
	if (!matrixelem)
		return name;
	DcSignal *psignal = m_controller->getSignal(matrixelem->src());
	if (psignal != nullptr) {
		QString templatestr = "(%1) %2";
		name = templatestr.arg(QString::number(psignal->internalId()), psignal->name());
	}
	return name;
}

bool OutputMatrixDelegate::isSignalBusy(BindMatrixElem * element) const
{
	DcSignal *signal = dynamic_cast<DcSignal *>(element);
	if (!signal)
		return false;
	
	return m_controller->matrix_signals()->existDst(signal->index());
}

bool OutputMatrixDelegate::removeElementBind(BindMatrixElem * element)
{
	DcSignal *signal = dynamic_cast<DcSignal *>(element);
	if (!signal)
		return false;

	DcMatrixElementSignal* bindPhisic = m_controller->matrix_signals()->getDst(signal->index());
	if (!bindPhisic)
		return false;

	// Проверяем, была ли эта привязка использована в гибкой логике
	int32_t sourseIndex = bindPhisic->src();
	DcMatrixElementAlgCfc *bindFlexLogic = m_controller->matrix_cfc()->getDst(sourseIndex);	// В гибкой логике это должен быть выход	
	if (bindFlexLogic) {	// Если такой сигнал есть
		QString info = "Привязка уже используется в алгоритме гибкой логики! \nДля удаления переназначьте используемый сигнал.. ";
		MsgBox::error(info);
		return false;
	}
	
	DcSignal * sourseSignal = m_controller->getSignal(sourseIndex);	// Источник виртуальный или физический вход
	if(sourseSignal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)
		removeTagFromName(sourseSignal, signal->name());	// Если есть таг в имени, удаляем его

	DcMatrixElementSignal *prem = m_controller->matrix_signals()->getDst(signal->index());	// Удаляем только из этой таблицы, в алгоритмах останется виртуальный сигнал
	if (prem == nullptr)
		return false;
	m_controller->matrix_signals()->remove(prem->src(), prem->dst());

	return true;
}

IBindDialog * OutputMatrixDelegate::createDialog(BindMatrixElem* element)
{
	DcSignal *signal = dynamic_cast<DcSignal *>(element);
	if (!signal)
		return nullptr;
	return new OutputBindDialog(m_controller, signal);
}
