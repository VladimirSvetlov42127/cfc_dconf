#include "AlgMatrixDelegate.h"

#include <gui/forms/table_of_bindings/dialogs/signal_names_functions.h>
#include <gui/forms/table_of_bindings/dialogs/AlgBindDialog.h>

using namespace Dpc::Gui;


AlgMatrixDelegate::AlgMatrixDelegate(std::map<QStandardItem*, BindMatrixElem*>* mmap, DcController *controller, QObject * parent)
	: IMatrixDelegate(mmap, controller, parent)
{
}

QString AlgMatrixDelegate::getBusySignalName(BindMatrixElem* element) const
{
	QString name;
	DcAlgIO *alg = dynamic_cast<DcAlgIO *>(element);
	if (!alg)
		return name;
	
	DcMatrixElementAlg* matrixSourse = m_controller->matrix_alg()->get(alg->index());	// Алгоритмы только как входы!
	if (!matrixSourse)
		return name;

	DcSignal *psignal = m_controller->getSignal(matrixSourse->dst());
	if (psignal != nullptr) {
		QString templatestr = "(%1) %2";
		name = templatestr.arg(QString::number(psignal->internalId()), psignal->name());
	}

	return name;
}

bool AlgMatrixDelegate::isSignalBusy(BindMatrixElem* element) const
{
	DcAlgIO *alg = dynamic_cast<DcAlgIO *>(element);
	if (!alg)
		return false;

	if (m_controller == nullptr)
		return false;

	if (m_controller->matrix_alg()->existSrc(alg->index()))
		return true;
	return false;
}

bool AlgMatrixDelegate::removeElementBind(BindMatrixElem* element)
{
	DcAlgIO *alg = dynamic_cast<DcAlgIO *>(element);
	if (!alg)
		return false;

	DcMatrixElementAlg* matrixelem = m_controller->matrix_alg()->get(alg->index());	// Получаем элемент по внутреннему индексу алгоритма
	if (!matrixelem)
		return false;

	int32_t destSignalIndex = matrixelem->dst();	// Тот, который выбрали
	int32_t algSigNumber = alg->index();	// Это тот сигнал, к которому привязываем

	// Проверяем, была ли эта привязка использована в гибкой логике
	if (alg->direction() == IO_DIRECTION_OUTPUT) {
		DcMatrixElementAlgCfc *pmatrixOut = m_controller->matrix_cfc()->getDst(destSignalIndex);	// В гибкой логике это вход, но в таблице matrix_cfc - dst
		if (pmatrixOut) {	// Если такой сигнал есть
			QString info = "Привязка уже используется в алгоритме гибкой логики! \nДля удаления переназначьте используемый сигнал. ";
			MsgBox::error(info);
			return false;
		}
	}

	DcSignal *destSignal = m_controller->getSignal(destSignalIndex);	// Получаем сигнал как назначение
	if (!destSignal) {
		MsgBox::error("Ошибка обнаружения сигнала! Привязка будет удалена!");
		DcMatrixElementAlg* matrixelemOut = m_controller->matrix_alg()->getDst(destSignalIndex);
		m_controller->matrix_alg()->removeByDest(destSignalIndex);	// Удаляем из таблицы встроенных алгоритмов
		return false;
	}

	if (alg->direction() == IO_DIRECTION_OUTPUT) {
		DcMatrixElementAlg* matrixelemOut = m_controller->matrix_alg()->getDst(destSignalIndex);
		while (matrixelemOut) {
			m_controller->matrix_alg()->removeByDest(destSignalIndex);	// Удаляем из таблицы встроенных алгоритмов
			matrixelemOut = m_controller->matrix_alg()->getDst(destSignalIndex);
		}

		DcMatrixElementSignal *psignalmatrix = m_controller->matrix_signals()->get(destSignalIndex);	// Если был физический выход, он удалится из матрицы сигналов
		while (psignalmatrix) {
			m_controller->matrix_signals()->removeBySourse(destSignalIndex);
			psignalmatrix = m_controller->matrix_signals()->get(destSignalIndex);
		}

		if ((destSignal->type() == DEF_SIG_TYPE_DISCRETE) && (destSignal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)) {

			// Обновляем имя виртуального входа
			int num = m_controller->virtualDiscreteIndex(destSignal) + 1;
			QString newName = VIRTUAL_DEFAULT_NAME;
			if (num)
				newName.append(QString(" %1").arg(num));
			destSignal->updateName(newName);

		}
	}
	else if (alg->direction() == IO_DIRECTION_INPUT) {

		if (destSignal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
			QString newName = removeAlgInput(alg->name(), destSignal->name());
			destSignal->updateName(newName);
		}

		m_controller->matrix_alg()->removeBySourse(algSigNumber);	// Удаляем из таблицы встроенных алгоритмов

	}

	return true;
}


IBindDialog * AlgMatrixDelegate::createDialog(BindMatrixElem* element)
{
	DcAlgIO *alg = dynamic_cast<DcAlgIO *>(element);
	if (!alg)
		return nullptr;
	return new AlgBindDialog(m_controller, alg);
}

