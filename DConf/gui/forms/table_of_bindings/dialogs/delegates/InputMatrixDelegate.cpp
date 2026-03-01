#include "InputMatrixDelegate.h"

#include <gui/forms/table_of_bindings/dialogs/signal_names_functions.h>
#include <gui/forms/table_of_bindings/dialogs/InputBindDialog.h>

InputMatrixDelegate::InputMatrixDelegate(std::map<QStandardItem*, BindMatrixElem*>* mmap, DcController *controller, QObject * parent)
	: IMatrixDelegate(mmap, controller, parent)
{
}

QString InputMatrixDelegate::getBusySignalName(BindMatrixElem* element) const
{	
	DcSignal *signalInput = dynamic_cast<DcSignal*>(element);	// Со входа можно подавать сигналы на несколько разных выходов!
	if (!signalInput)
		return QString();

	QString name;

	if (m_controller->matrix_alg()->existDst(signalInput->index())) {	// Если есть привязка к встроенным алгоритмам

		size_t algCount = m_controller->algs_internal()->size();	// Проходим по всем алгоритмам и ищем используемый вход		
		for (size_t i = 0; i < algCount; i++) {	// Создает для каждого внутреннего алгоритма подмодель
			DcAlgInternal *palg = m_controller->algs_internal()->get(i);		// Получаем конкретный внутренний алгоритма

			size_t algIOCont = palg->ios()->size();	// Кол-во входов/выходов в нем
			bool hasAlgName = false;
			for (size_t i = 0; i < algIOCont; i++) {	// Проходим по всем входам/выходам
				DcAlgIOInternal *pios = palg->ios()->get(i);	// Здесь получаем инфо о входе/выходе данного элемента, есть или нет

				DcMatrixElementAlg* matrixAlg = m_controller->matrix_alg()->get(pios->index());	// Проверяем, есть ли связь с этим входом/выходом
				if (!matrixAlg)
					continue;

				if (signalInput->index() == matrixAlg->dst()) {
					if (hasAlgName)	// Если в одном алгоритме несколько привязок
						name += ", " + pios->name();
					else {
						if (name.isEmpty()) 	// Если первый алгоритм
							name = "(" + palg->name() + ") -> " + pios->name();
						else
							name += ", (" + palg->name() + ") -> " + pios->name();
						hasAlgName = true;
					}
				}

				//if (pios->index() == matrixAlg->src()) {
				//	name = "(" + palg->name() + ") -> " + pios->name();
				//	//name = addOutputTag(pios->name(), palg->name());
				//	break;
				//}
			}
		}
	}

	QString templatestr = "(%1) %2";
	if (m_controller->matrix_signals()->existSrc(signalInput->index())) {	// Если привязано к физическому выходу
		uint64_t bindCount = m_controller->matrix_signals()->size();

		for (uint32_t bind = 0; bind < bindCount; bind++) {	// Проходим по всем существующим связям
			DcMatrixElementSignal* matrixSignal = m_controller->matrix_signals()->get(bind);	// Берем по номеру
			if (matrixSignal->src() == signalInput->index()) {
				DcSignal* physOutput = m_controller->getSignal(matrixSignal->dst());
				if (!physOutput)
					continue;
				if (name.isEmpty())
					name = templatestr.arg(QString::number(physOutput->internalId()), physOutput->name());
				else
					name += ", " + templatestr.arg(QString::number(physOutput->internalId()), physOutput->name());
			}
		}
	}

	return name;
}

bool InputMatrixDelegate::isSignalBusy(BindMatrixElem* element) const
{
	DcSignal *signalInput = dynamic_cast<DcSignal*>(element);
	if (!signalInput)
		return false;

	if (m_controller == nullptr)
		return false;
	
	if (m_controller->matrix_alg()->existDst(signalInput->index()) || m_controller->matrix_signals()->existSrc(signalInput->index())) // Мможет быть использован в привязке к алгоритмам или к другим
		return true;
	return false;
}

bool InputMatrixDelegate::removeElementBind(BindMatrixElem* element)
{
	DcSignal *signalInput = dynamic_cast<DcSignal*>(element);
	if (!signalInput)
		return false;

	if (!m_controller)
		return false;

	DcMatrixElementAlg* matrixAlg = m_controller->matrix_alg()->getDst(signalInput->index());
	DcMatrixElementSignal* matrixSignal = m_controller->matrix_signals()->get(signalInput->index());	
	while (matrixAlg) {
		m_controller->matrix_alg()->remove(matrixAlg->src(), matrixAlg->dst());
		matrixAlg = m_controller->matrix_alg()->getDst(signalInput->index());
	}
	while (matrixSignal) {
		m_controller->matrix_signals()->remove(matrixSignal->src(), matrixSignal->dst());
		matrixSignal = m_controller->matrix_signals()->get(signalInput->index());
	}

	return true;
}

IBindDialog * InputMatrixDelegate::createDialog(BindMatrixElem* element)
{
	DcSignal *signal = dynamic_cast<DcSignal*>(element);
	if (!signal)
		return nullptr;
	return new InputBindDialog(m_controller, signal);
}
