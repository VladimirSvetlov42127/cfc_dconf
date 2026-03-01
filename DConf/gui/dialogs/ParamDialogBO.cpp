#include "ParamDialogBO.h"

#include <data_model/dc_data_manager.h>
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

#include <gui/forms/DcSignalManager.h>

using namespace std;
using namespace Dpc::Gui;

ParamDialogBO::ParamDialogBO(QList<FlexLogic::NodeParam> &params, DcSignalManager *signalManager, QWidget *parent) : QDialog(parent), m_signalManager(signalManager), m_params(params), m_ioid(0)
{
	//	Получение контроллера
	_controller = signalManager->device();

	setWindowIcon(QIcon(":/icons/24/exchange.png"));
	ui.setupUi(this);

	model = new QStandardItemModel(ui.treePhisOutput);
	ui.treePhisOutput->setModel(model);
	ui.treePhisOutput->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.treePhisOutput->setSelectionMode(QAbstractItemView::SingleSelection);

	m_virt_dout_ind = 0;
	fillTree();
	ui.treePhisOutput->collapseAll();
	this->setStyleSheet("background-color: white;");
	for (auto &it : m_params) {
		if (it.index.contains("io_id")) {
			if (it.value.toInt() > 0)
				m_ioid = it.value.toInt();	// Это номер текущего выхода в алгоритме
			//return;
		}
		else if (it.index.contains("name"))
			ui.edTitle->setText(it.value.toString());
		else if (it.index.contains("signal"))
			m_virtInternalId = it.value.toInt();
	}

	bool ok = connect(ui.treePhisOutput, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(dblclicked(const QModelIndex&)));
	Q_ASSERT(ok);

}

ParamDialogBO::~ParamDialogBO()
{
}

void ParamDialogBO::onSaveChanges() {
	if (m_ioid > 0) {
		//removeOldDout();	// Удаляем старый
		replaceOut();
	}
	else {

		createNewDout();	// Создается новый выход	
	}
	return;
}

void ParamDialogBO::onCancel() {
	this->done(QDialog::Rejected);
}

void ParamDialogBO::onTreeSelection() {
	QModelIndex index = ui.treePhisOutput->currentIndex();
	if (!index.isValid()) {
		return;
	}

	DcSignal* pitem = getItemDataTree(index);
	if (pitem == nullptr) {
		return;
	}

	int32_t ioid;
	if (m_ioid > 0)
		ioid = m_ioid;
    else
        ioid = m_signalManager->getNewCfcIo();
	QString titletmp = "%1 (%2:%3) %4";
	QString res = titletmp.arg(pitem->internalId()).arg(QString::number(m_signalManager->alg()->index()), QString::number(ioid), pitem->name());
	ui.edTitle->setText(res);
	return;
}

void ParamDialogBO::dblclicked(const QModelIndex& element) {
	onSaveChanges();
}

void ParamDialogBO::fillTree()
{
	formedListBySignalType_t list = m_signalManager->getSignalList(DefSignalDirection::DEF_SIG_DIRECTION_OUTPUT);
	QStandardItem *proot = model->invisibleRootItem();

	for (int sigTypeInd = 0; sigTypeInd < list.size(); sigTypeInd++) {
		QStandardItem* pitemSigType = new QStandardItem(list.at(sigTypeInd).first);	// Создаем элемент с названием типа
		pitemSigType->setEditable(false);
		pitemSigType->setIcon(QIcon(":/icons/24/folder_project.png"));
		proot->appendRow(pitemSigType);

		QString nametmp = "(%1) %2";
		QList<DcSignal*> signalListCurType = list.at(sigTypeInd).second;
		for (int signalInd = 0; signalInd < signalListCurType.size(); signalInd++) {
			DcSignal *curSignal = signalListCurType.at(signalInd);
			QString name = nametmp.arg(QString::number(curSignal->internalId()), curSignal->name());
			QStandardItem* pitem = new QStandardItem(name);
			pitem->setEditable(false);
			pitem->setIcon(QIcon(":icons/24/signal_out.png"));
			m_assocTableTree.insert(std::make_pair(pitem, curSignal));
			pitemSigType->appendRow(pitem); }
	}

	return;
}


DcSignal * ParamDialogBO::getItemDataTree(QModelIndex &index)
{
	QStandardItem *pitem = model->itemFromIndex(index);
	if (pitem == nullptr)
		return nullptr;
	auto search = m_assocTableTree.find(pitem);
	if (search == m_assocTableTree.end())
		return nullptr;

	return search->second;
}

void ParamDialogBO::replaceOut() {
	DcController *pcontr = m_signalManager->device();

	//Получаем данные от выбранного выхода
	QModelIndex index = ui.treePhisOutput->currentIndex();	// Получаем выбранный элемент дерева модели
	if (!index.isValid()) {
		MsgBox::error("Некорректная строка выбрана в дереве сигналов");
		return;
	}
	QString blockTitle = ui.edTitle->text();	// Получаем заполненное имя алгоритма 
	if (blockTitle.isEmpty()) {
		MsgBox::error("Имя выхода алгоритма не может быть пустым");
		return;
	}
	DcSignal* signal = getItemDataTree(index);	// Получаем сигнал, соответствующий этому индексу
	if (!signal) {
		signal = m_signalManager->getSignal(m_ioid);
		if (!signal) {
			MsgBox::error("Выбранный сигнал не найден в классификаторе");
			return;
		}

		signal->updateName(blockTitle);
	} else
		bool isOk = m_signalManager->signalReplaced(m_ioid, signal, blockTitle);

	for (auto &it : m_params) {
		if (it.index.contains("name"))
			it.value = blockTitle;
		else if (it.index.contains("signal"))
			it.value = signal->internalId();
	}	

	this->done(QDialog::Accepted);
}

void ParamDialogBO::createNewDout() {

	//Получаем данные от выбранного выхода
	QModelIndex index = ui.treePhisOutput->currentIndex();	// Получаем выбранный элемент дерева модели
	if (!index.isValid()) {
		MsgBox::error("Некорректная строка выбрана в дереве сигналов");
		return;
	}
	DcSignal* pitem = getItemDataTree(index);	// Получаем сигнал, соответствующий этому индексу
	if (pitem == nullptr) {
		MsgBox::error("Выбранный сигнал не найден в классификаторе");
		return;
	}
	QString blockTitle = ui.edTitle->text();	// Получаем заполненное имя алгоритма 
	if (blockTitle.isEmpty()) {
		MsgBox::error("Имя выхода алгоритма не может быть пустым");
		return;
	}

	int32_t ioid(0), alg_ind(0);
	bool isOk = m_signalManager->signalBusy(pitem, ioid, alg_ind, blockTitle);
	if (!isOk) {
		return;
	}
	// Сохранение данных в элементе
	for (auto &it : m_params) {
		if (it.index.contains("io_id"))
			it.value = ioid;
		else if (it.index.contains("name"))
			it.value = blockTitle;
		else if (it.index.contains("signal"))
			it.value = pitem->internalId();
		else if (it.index.contains("alg_pin"))
			it.value = alg_ind + 1;
	}

	this->done(QDialog::Accepted);
}

