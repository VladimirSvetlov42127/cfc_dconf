#include "DcSignalManager.h"

#include <qregularexpression.h>

#include <dpc/gui/dialogs/msg_box/MsgBox.h>
#include <QDebug>

using namespace Dpc::Gui;

namespace {
}

#define CFC_ALG_IO_SIZE 32

DcSignalManager::DcSignalManager(DcController *device, DcAlgCfc *alg, QObject *parent) :
	QObject(parent),
	m_device(device),
	m_alg(alg)
{
}

DcController * DcSignalManager::device() const
{
	return m_device;
}

DcAlgCfc * DcSignalManager::alg() const
{
	return m_alg;
}

formedListBySignalType_t DcSignalManager::getSignalList(DefSignalDirection dir)
{
	DcController *controller = m_device;

	QList<DcSignal*> phisOutputSignals;
	QList<DcSignal*> indicatorsOutputSignals;
	QList<DcSignal*> freeVirtualOutputSignals;

	QList<DcSignal*> phisInputSignals;
	QList<DcSignal*> logicInputSignals;
	QList<DcSignal*> customInputSignal;
	QList<DcSignal*> remoteInputSignal;
	QList<DcSignal*> ledAdInputSignal;
	int virtualDiscretInputIdx = -1;

	for (auto signal: controller->getSignalList()) {
		if ((signal->type() != DEF_SIG_TYPE_DISCRETE)) continue;
		if (signal->direction() == DEF_SIG_DIRECTION_INPUT && signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)
			virtualDiscretInputIdx++;

		if (dir == DEF_SIG_DIRECTION_OUTPUT) {

			// Формирование списка незанятых виртуальных сигналов
			if ((signal->direction() == DEF_SIG_DIRECTION_INPUT) && (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)) {
				bool existInFlexLogic = controller->matrix_cfc()->existDst(signal->index());	// В гибкой логике
				bool existInPhysicBingSrc = controller->matrix_signals()->existSrc(signal->index()); // Алгоритм к физическому для физ входов
				bool existInPhysicBingDest = controller->matrix_signals()->existDst(signal->index()); // Алгоритм к физическому для физ входов
				bool existInAlg = controller->matrix_alg()->existDst(signal->index());	// Алгоритм к виртуальному для физ входов
				bool isConnDiscret = controller->isConnectionDiscret(signal);
				bool isVirtualFunction0 = controller->isVirtualFunctionParamValue(DcController::VDIN_CONTROL, virtualDiscretInputIdx);
				bool isVirtualFunction1 = controller->isVirtualFunctionParamValue(DcController::FIX_VDIN, virtualDiscretInputIdx);
				bool isVirtualFunction2 = controller->isVirtualFunctionParamValue(DcController::VDIN_EVENT, virtualDiscretInputIdx);
				bool isVirtualFunction = isVirtualFunction0 || isVirtualFunction1 || isVirtualFunction2;

				//	Дополнительные проверки
				bool is_compare = CheckAinsCompare(signal->internalId());
//				bool is_discrete = CheckDiscrete(signal->internalId());

				//	Проверяем, что сигнал нигде не используется
				bool signal_exist = existInFlexLogic || existInPhysicBingSrc || existInPhysicBingDest || existInAlg || isConnDiscret || 
                    isVirtualFunction || is_compare; // || is_discrete;

				//	Добавляем в список свободных виртуальных входов
				if (!signal_exist && !signal->isFixed()) freeVirtualOutputSignals.append(signal); }

			if (signal->direction() != DEF_SIG_DIRECTION_OUTPUT) continue;
			if (signal->subtype() == DEF_SIG_SUBTYPE_PHIS) {
				if (controller->matrix_signals()->existDst(signal->index())) {
					continue; }
				phisOutputSignals.append(signal);
			} else if (signal->subtype() == DEF_SIG_SUBTYPE_LED_AD) {
				if (controller->matrix_signals()->existDst(signal->index())) continue;
				indicatorsOutputSignals.append(signal);	}
		}
		else if (dir == DEF_SIG_DIRECTION_INPUT) {
			if (signal->direction() != DEF_SIG_DIRECTION_INPUT) {
				continue;
			}

			if (signal->subtype() == DEF_SIG_SUBTYPE_PHIS) {
				if (!isAlreadyBinded(signal))
					phisInputSignals.append(signal);
			}
			else if (signal->subtype() == DEF_SIG_SUBTYPE_LOGIC) {
				logicInputSignals.append(signal);
			}
			else if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
				bool existInAlgLikeDst = controller->matrix_alg()->existDst(signal->index());
				bool existInFlexLogic = controller->matrix_cfc()->existDst(signal->index());
				bool existInPhysicBind = controller->matrix_signals()->existDst(signal->index());
				bool isConnDiscret = controller->isConnectionDiscret(signal);
				bool isVirtualFunction0 = controller->isVirtualFunctionParamValue(DcController::VDIN_CONTROL, virtualDiscretInputIdx);
				bool isVirtualFunction1 = controller->isVirtualFunctionParamValue(DcController::FIX_VDIN, virtualDiscretInputIdx);
				bool isVirtualFunction2 = controller->isVirtualFunctionParamValue(DcController::VDIN_EVENT, virtualDiscretInputIdx);
				bool isVirtualFunction = isVirtualFunction0 || isVirtualFunction1 || isVirtualFunction2;

				//	Дополнительные проверки
				bool is_compare = CheckAinsCompare(signal->internalId());
//				bool is_discrete = CheckDiscrete(signal->internalId());

				//	Проверяем, что сигнал нигде используется
				if (!existInAlgLikeDst && !existInFlexLogic && !existInPhysicBind && !signal->isFixed() && !isConnDiscret && !isVirtualFunction &&
                    !is_compare /* && !is_discrete*/) continue;
				customInputSignal.append(signal);
			}
			else if (signal->subtype() == DEF_SIG_SUBTYPE_REMOTE) {
				remoteInputSignal.append(signal);
			}
			else if (signal->subtype() == DEF_SIG_SUBTYPE_LED_AD) {
				ledAdInputSignal.append(signal);
			}
		}
	}

	formedListBySignalType_t result;
	auto addToResult = [&result](const QString &name, const QList<DcSignal*> &sigList) {
		if (!sigList.isEmpty())
			result.append(qMakePair(name, sigList));
	};

	if (dir == DEF_SIG_DIRECTION_INPUT) {
		addToResult("Физические входы", phisInputSignals);
		addToResult("Логические входы", logicInputSignals);
		addToResult("Назначенные виртуальные входы", customInputSignal);
		addToResult("Внешние входы", remoteInputSignal);
		addToResult("Входы на внутреннем АЦП", ledAdInputSignal);
	}
	else if (dir == DEF_SIG_DIRECTION_OUTPUT) {
		addToResult("Физические выходы", phisOutputSignals);
		addToResult("Индикаторы", indicatorsOutputSignals);
		addToResult("Пользовательские выходы", customInputSignal);
		addToResult("Виртуальные входы", freeVirtualOutputSignals);
	}

	return result;
}

bool DcSignalManager::signalFree(DcSignal *signal) {

	int32_t signalIndex = signal->index();
	DcController *pcontr = m_device;
	DcAlgCfc *palg = m_alg;
	if (signal->direction() == DEF_SIG_DIRECTION_OUTPUT) {
		// Удаляем привязки к реальным выходам	//remove link to dout
		DcMatrixElementSignal *psignalmatrix = pcontr->matrix_signals()->getDst(signalIndex);
		int32_t virSignalInd = 0;
		if (psignalmatrix != nullptr) {
			virSignalInd = psignalmatrix->src();
			pcontr->matrix_signals()->removeByDest(signalIndex);
		}

		// Удаляяем из таблиц гибкой логики
		DcMatrixElementAlgCfc *poldmatrix = pcontr->matrix_cfc()->getDst(virSignalInd);
		if (poldmatrix == nullptr)
			return false;
		palg->ios()->removeById(poldmatrix->src());	// Удаляем из списков алгоритмов

		// Удаляем из таблицы гибкой логики
		pcontr->matrix_cfc()->removeByDest(signalIndex);

		// Проверяем связи с внутренними алгоритмами
		DcMatrixElementAlg* matrixelem = pcontr->matrix_alg()->getDst(signalIndex);	// Проверяем, если такой сигнал в таблице привязки внутренних алгоритмов
		if (matrixelem) {	// Если есть
			pcontr->matrix_alg()->removeByDest(signalIndex);	// Удаляем старый
		}

		pcontr->getSignal(virSignalInd)->updateName(VIRTUAL_DEFAULT_NAME); //update vdin name
	}
	else if (signal->direction() == DEF_SIG_DIRECTION_INPUT) {
		DcMatrixElementAlgCfc *poldmatrix = pcontr->matrix_cfc()->getDst(signalIndex);
		if (poldmatrix != nullptr) {
			palg->ios()->removeById(poldmatrix->src());
			pcontr->matrix_cfc()->removeByDest(signalIndex);
		}

		if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {	// Виртуальный сигнал может быть назначен на выход и использоваться в нескольких входах во встроенных алгоритмах
			bool isUsed = false;
			
			DcMatrixElementAlg* matrixelem = pcontr->matrix_alg()->getDst(signalIndex);	// Проверяем, если такой сигнал в таблице привязки внутренних алгоритмов
			if (matrixelem) {
				isUsed = true;
				//pcontr->matrix_alg()->removeByDest(signalIndex);	// Удаляем старый
				//matrixelem = pcontr->matrix_alg()->getDst(signalIndex);
			}
			// Удаляем привязки к реальным выходам	//remove link to dout
			DcMatrixElementSignal *psignalmatrix = pcontr->matrix_signals()->get(signalIndex);
			if (!isUsed) {	// Если не используется в таблице привязки, удаляем
				while (psignalmatrix) {
					pcontr->matrix_signals()->remove(signalIndex, psignalmatrix->dst());
					psignalmatrix = pcontr->matrix_signals()->get(signalIndex);
				}				
			}
			if (isUsed) {
				QString sigName = signal->name();
				int pos = sigName.indexOf(')') + 2;
				if (pos > 0) {
					signal->updateName(sigName.right(sigName.size() - pos));
				}

			}
			else {				
				QString signalNewName = VIRTUAL_DEFAULT_NAME;
				auto idx = pcontr->virtualDiscreteIndex(signal);
				if (idx > -1)
					signalNewName.append(QString(" %1").arg(idx + 1));
				signal->updateName(signalNewName); //update vdin name
			}
		}

	}

	return true;
}

bool DcSignalManager::signalBusy(DcSignal *signal, int32_t &ioid, int32_t &algpos, QString &virtName) {

	DcController *pcontr = m_device;
	if (!signal) return false;

	int32_t signal_id = signal->index();

    // Ищем свободные места для выхода
    ioid = getNewCfcIo();	// Получаем следующий доступный номер для входа/выхода, внутренний для ВСЕХ
    algpos = getFreeCfcAlgPin(m_alg->index());	// для каждого алгоритма доступно CFC_ALG_IO_SIZE пинов, котрые можно занять
	if (algpos < 0) {
		MsgBox::error("Лимит входов/выходов исчерпан");
		return false;
	}

	bool nameHasNumber = virtName.contains('(') && virtName.contains(')');
	QRegularExpression reg("^(*:*) ");
	bool signalNameHasNumber = signal->name().contains(reg);
	if (!nameHasNumber && !signalNameHasNumber) {
		QString titletmp = "(%1:%2) %3";
		if (virtName.isEmpty() || virtName == "") {
			virtName = titletmp.arg(QString::number(m_alg->index()), QString::number(ioid), signal->name());
		}
		else {
			virtName = titletmp.arg(QString::number(m_alg->index()), QString::number(ioid), virtName);
		}
	}
	else {
		if (virtName.isEmpty()) {
			virtName = signal->name();
		}		
	}

	if (signal->direction() == DefSignalDirection::DEF_SIG_DIRECTION_OUTPUT) {	// Создание входов и выходов по-разному

		//// Создаем выход алгоритма - привязываем в виртуальному входу - привязываем к реальному выходу
		// Создается новый выход для текущего алгоритма
		DcAlgCfc *pcfcalg = m_alg;	// Получаем алгоритм
		if (pcfcalg == nullptr) {
			MsgBox::error("Алгоритм с данным номером не существует в базе");
			return false;
		}
		DcAlgIOCfc *pnewio = new DcAlgIOCfc(m_device->index(), ioid, m_alg->index(), algpos, IO_DIRECTION_OUTPUT, signal->name());	// Создаем выход
		pcfcalg->ios()->add(pnewio, false);	// Добавляем в алгоритм новый выход

		// Ищем свободный виртуальный вход
        DcSignal *pvdin = getFreeVDin(); 	// Возвращает первый незанятый виртуальный выход
		if (pvdin == nullptr) {
			MsgBox::error("Не найдено свободных виртуальных входов");
			return false;
		}
		pvdin->updateName(virtName);	// Обновляем имя выхода

		//// Заносим информацию в таблицы
		// Гибкая логика. Источник - выход алгоритма ioid, цель - уникальный индекс виртуального входа
		DcMatrixElementAlgCfc *pmatrix = new DcMatrixElementAlgCfc(m_device->index(), ioid, pvdin->index(), "");
		pcontr->matrix_cfc()->add(pmatrix, false);

		if (signal->subtype() != DEF_SIG_SUBTYPE_VIRTUAL) {	// Если привязка осуществляется к физическому, дополнительно создаем связи
			// link to dout signal
			// Таблица привязки сигналов. Источник - уникальный индекс виртуального входа, выход алгоритма - иднекс выбранного сигнала
			DcMatrixElementSignal *poutmatrix = new DcMatrixElementSignal(m_device->index(), pvdin->index(), signal->index(), "");
			pcontr->matrix_signals()->add(poutmatrix, false);

			//// Удаляем из таблицы встроенных алгоритмов (это если вдруг есть связь)
			//DcMatrixElementAlg* matrixelem = pcontr->matrix_alg()->getDst(m_virt_dout_ind);	// Проверяем, если такой сигнал в таблице привязки внутренних алгоритмов
			//if (matrixelem) {	// Если есть
			//	int32_t oldSource = matrixelem->src();	// Сохраняем номер для перенезначения
			//	pcontr->matrix_alg()->remove(oldSource, m_virt_dout_ind);	// Удаляем старый

			//	DcMatrixElementAlg* newMatrixElem = new DcMatrixElementAlg(oldSource, pvdin->index(), "");	// Создаем новый
			//	newMatrixElem->setUid(pcontr->index());	// Нужно для создания
			//	pcontr->matrix_alg()->add(newMatrixElem, false);
			//}
		}

	}
	else {	// Создание входа

		DcAlgCfc *pcfcalg = m_alg;
		if (pcfcalg == nullptr) {
			MsgBox::error("Алгоритм с данным номером не существует в базе");
			return false;
		}
		DcAlgIOCfc *pnewio = new DcAlgIOCfc(m_device->index(), ioid, m_alg->index(), algpos, IO_DIRECTION_INPUT, signal->name());
		pcfcalg->ios()->add(pnewio, false);

		DcMatrixElementAlgCfc *pmatrix = new DcMatrixElementAlgCfc(m_device->index(), ioid, signal->index(), "");
		pcontr->matrix_cfc()->add(pmatrix, false);

		if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
			signal->updateName(virtName);
		}

	}

	return true;
}

bool DcSignalManager::signalReplaced(int32_t signalIOID, DcSignal *signalNew, QString name) {
	if (signalIOID == 0 || !signalNew) 
		return false;

	int32_t ioid;
	// Виртуальный сигнал, назначаемый для выхода имеет тип INPUT!!!
	if (signalNew->direction() == DEF_SIG_DIRECTION_OUTPUT || signalNew->subtype() == DEF_SIG_SUBTYPE_VIRTUAL) {
		///// Получаем текущий 
		DcController *pcontr = m_device;
		DcAlgCfc *palg = m_alg;

		DcAlgIOCfc *pio = palg->ios()->getById(signalIOID);	// Получаем характеристики самого выхода  m_io_uid; = input/output id uniq per system
		if (pio == nullptr)
			return false;

		ioid = pio->index();	// Сохраняем индексы старого значения
		//palg->ios()->removeById(ioid);	// Удаляем старый из входов/выходов данного алгоритма

		//// Удаляем из двух таблиц, чтобы заново обновить имена
		// Удаляем старые и создаем новые в таблице гибкой логики
		long m_virt_dout_ind = 0;	// Сохраняем индекс виртуального входа, чтобы по нему определить старый физический выход
		DcMatrixElementAlgCfc *pmatrix = pcontr->matrix_cfc()->get(ioid);
		if (pmatrix) {
			m_virt_dout_ind = pmatrix->dst();	// получаем индекс виртуального входа
			//pcontr->matrix_cfc()->removeBySourse(ioid);
		}
		else return false;

		DcSignal * oldSignal = pcontr->getSignal(m_virt_dout_ind);
		if (!oldSignal) {
			MsgBox::error("Ошибка! Не существует старого сигнала при попытке его обновить!");
			return false;
		}

		// Смотрим, меняем ли мы тип привязки с виртуального на физический и обратно
		if (signalNew->subtype() != DEF_SIG_SUBTYPE_VIRTUAL) {
			oldSignal->updateName(name);
			// Удаляем старые и создаем новые в таблице привязки, но не трогаем привязку к виртуальному сигналу
			DcMatrixElementSignal* oldBoundFromVirtToPhys = pcontr->matrix_signals()->get((int32_t) m_virt_dout_ind);
			if (oldBoundFromVirtToPhys) {
				pcontr->matrix_signals()->removeBySourse(m_virt_dout_ind);	// Удаляем старый
			}

			DcMatrixElementSignal *newVirtToPhisBound = new DcMatrixElementSignal(m_device->index(), oldSignal->index(), signalNew->index(), "");
			pcontr->matrix_signals()->add(newVirtToPhisBound, false);
		}
		else {	// Если новый вид - виртуальный
			QString oldSignalNewName = VIRTUAL_DEFAULT_NAME;
			auto idx = pcontr->virtualDiscreteIndex(oldSignal);
			if (idx > -1)
				oldSignalNewName.append(QString(" %1").arg(idx + 1));
			oldSignal->updateName(oldSignalNewName); // Старый сигнал переопределяет имя
			signalNew->updateName(name);	// На новом тоже меняем имя, к виртуальному добавляем индексы алгоритма напр. (1:1)
			// Удаляем связь с реальным (важно, если старый сигнал был физический
			DcMatrixElementSignal* oldBoundFromVirtToPhys = pcontr->matrix_signals()->get((int32_t) m_virt_dout_ind);
			if (oldBoundFromVirtToPhys) {
				pcontr->matrix_signals()->removeBySourse(m_virt_dout_ind);	// Удаляем старый
			}


			// Удаляем из таблицы встроенных алгоритмов (это если вдруг есть связь)
			DcMatrixElementAlg* matrixelem = pcontr->matrix_alg()->getDst(m_virt_dout_ind);	// Проверяем, если такой сигнал в таблице привязки внутренних алгоритмов
			if (matrixelem) {	// Если есть
				int32_t oldSource = matrixelem->src();	// Сохраняем номер для перенезначения
				pcontr->matrix_alg()->remove(oldSource, m_virt_dout_ind);	// Удаляем старый

                DcMatrixElementAlg* newMatrixElem = new DcMatrixElementAlg(pcontr->index(), oldSource, signalNew->index(), "");	// Создаем новый
				pcontr->matrix_alg()->add(newMatrixElem, false);
			}

			// Удаляем старый и обновляем новый с новый виртуальным сигналом
			pcontr->matrix_cfc()->removeBySourse(ioid);
			DcMatrixElementAlgCfc *pmatrix = new DcMatrixElementAlgCfc(m_device->index(), ioid, signalNew->index(), "");
			pcontr->matrix_cfc()->add(pmatrix, false);
		}
	}
	else if (signalNew->direction() == DEF_SIG_DIRECTION_INPUT) {

		DcController *pcontr = m_device;
		DcAlgCfc *palg = m_alg;
		DcAlgIOCfc *pio = palg->ios()->getById(signalIOID);
		if (pio == nullptr)
			return false;

		if (name.isEmpty()) {
			MsgBox::error("Имя входа алгоритма не может быть пустым");
			return false;
		}

		if (pio->name().compare(name) != 0) {
			pio->updateName(name);
			pio->updateNamedb();
		}

		// Удаляем из матрицы алгоритмов
		DcMatrixElementAlgCfc *poldmatrix = pcontr->matrix_cfc()->get(signalIOID);
		if (poldmatrix != nullptr) {
			pcontr->matrix_cfc()->remove(signalIOID, poldmatrix->dst());
		}

		// Добавляем в матрицу алгоритмов
		DcMatrixElementAlgCfc *pmatrix = new DcMatrixElementAlgCfc(m_device->index(), signalIOID, signalNew->index(), "");
		pcontr->matrix_cfc()->add(pmatrix, false);

	}

	return true;
}

DcSignal * DcSignalManager::getSignal(int32_t ioid) const
{
	DcController *pcontr = m_device;
	DcAlgCfc *palg = m_alg;
	if (!palg)
		return nullptr;

	DcAlgIOCfc *pio = palg->ios()->getById(ioid);
	if (!pio)
		return nullptr;

	DcMatrixElementAlgCfc *pmatrix = pcontr->matrix_cfc()->get(pio->index());
	if (!pmatrix)
		return nullptr;

	return pcontr->getSignal(pmatrix->dst());
}

//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
//// Скопировано из ParamDialogBI.cpp
bool DcSignalManager::isAlreadyBinded(DcSignal * psignal) {
	if (!m_device->matrix_cfc()->existDst(psignal->index()))
		return false;

	DcAlgCfc *palg = m_alg;
	if (!palg)
		return false;

	for (size_t pos = 0; pos < palg->ios()->size(); pos++) {
		if (m_device->matrix_cfc()->exist(palg->ios()->get(pos)->index(), psignal->index()))
			return true;
	}
	return false;
}

// Добавлено
bool DcSignalManager::CheckAinsCompare(int signal_idx)
{
	int count = 0;
	while (DcParam* param = m_device->getParam(SP_AINCMP_OUTWORD_PARAM, count)) {
		uint32_t value = param->value().toUInt();
		if (value == signal_idx) return true;
		count++; }

	return false;
}

//bool DcSignalManager::CheckDiscrete(int signal_idx)
//{
//	int count = 0;
//	while (DcParam* param = m_device->getParam(SP_INDCON_OUTWORD_PARAM, count)) {
//		uint32_t value = param->value().toUInt();
//		if (value == signal_idx) return true;
//		count++; }

//	return false;
//}

int32_t DcSignalManager::getNewCfcIo() {

    DcController *pcontr = m_device;
    int maxcfcio = 0;
    for (size_t algpos = 0; algpos < pcontr->algs_cfc()->size(); algpos++) {
        DcAlgCfc *palg = pcontr->algs_cfc()->get(algpos);
        for (size_t iostep = 0; iostep < palg->ios()->size(); iostep++) {
            DcAlgIOCfc *pcfcio = palg->ios()->get(iostep);
            if (pcfcio->index() > maxcfcio)
                maxcfcio = pcfcio->index();
        }
    }
    return ++maxcfcio;
}

int32_t DcSignalManager::getFreeCfcAlgPin(int32_t algindex) {

    DcController *pcontr = m_device;
    DcAlgCfc *palg = pcontr->algs_cfc()->getById(algindex);	// Получаем алгоритм
    if (palg == nullptr)
        return -1;
    size_t iocount = palg->ios()->size();	// Сколько в алгоритме входов/выходов
    for (int i = 0; i < CFC_ALG_IO_SIZE; i++) {
        bool finded = false;
        for (size_t j = 0; j < iocount; j++) {
            DcAlgIOCfc *elem = palg->ios()->get(j);	// Получаем выход или выход алгоритма
            if (elem->pin() == i) {	//  Если совпал input/output number uniq per algorithm
                finded = true;
                break;
            }
        }
        if (!finded)
            return i;

    }
    return -1;
}

DcSignal *DcSignalManager::getFreeVDin() {	// Возвращает первый незанятый виртуальный выход?

    DcController *pcontr = m_device;
    //-----------------------
    //for (auto psignal: pcontroller->getSignalList(DEF_SIG_TYPE_DISCRETE, DEF_SIG_SUBTYPE_VIRTUAL)) {	// Проходит по списку сигналов
    //	if (pcontroller->matrix_alg()->existDst(psignal->index()))	// Проверяем, есть ли данный сигнал в матрице внутренних алгоритмов
    //		continue;
    //	if (pcontroller->matrix_cfc()->existDst(psignal->index()))	// Есть ли в матрице гибкой логики
    //		continue;
    //	if (pcontroller->matrix_signals()->existSrc(psignal->index()))	// Есть ли в таблице привязки сигналов
    //		continue;
    //-----------------------

    QList<DcSignal*> list = pcontr->getSignalList(DEF_SIG_TYPE_DISCRETE, DEF_SIG_SUBTYPE_VIRTUAL);
    DcSignal* psignal;
    int count = list.count() - 1;
    for (int i = count; i != 0; i--) {
        psignal = list.at(i);
        if (pcontr->matrix_alg()->existDst(psignal->index())) continue;
        if (pcontr->matrix_cfc()->existDst(psignal->index())) continue;
        if (pcontr->matrix_signals()->existSrc(psignal->index())) continue;
        return psignal;	}

    return nullptr;
}
