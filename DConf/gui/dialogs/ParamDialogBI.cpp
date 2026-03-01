#include "ParamDialogBI.h"
#include <data_model/dc_data_manager.h>
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

#include <gui/forms/DcSignalManager.h>

using namespace std;
using namespace Dpc::Gui;

ParamDialogBI::ParamDialogBI(QList<FlexLogic::NodeParam> &params, DcSignalManager *signalManager, QWidget *parent) :
    QDialog(parent), m_params(params), m_signalManager(signalManager), m_ioid(0)
{
	//	Получение контроллера
	_controller = signalManager->device();

	setWindowIcon(QIcon(":/icons/24/exchange.png"));
	ui.setupUi(this);
	model = new QStandardItemModel(ui.signalsTree);
	ui.signalsTree->setModel(model);
	ui.signalsTree->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.signalsTree->setSelectionMode(QAbstractItemView::SingleSelection);
	fillTree();
	ui.signalsTree->collapseAll();
	this->setStyleSheet("background-color: white;");
	for (auto &it : m_params) {
		if (it.index.contains("io_id")) {
			if (it.value.toInt() > 0)
				m_ioid = it.value.toInt();
			//return;
		}
		else if (it.index.contains("name"))
			ui.edTitle->setText(it.value.toString());
	}

	bool ok = connect(ui.signalsTree, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(dblclicked(const QModelIndex&)));
	Q_ASSERT(ok);
}

ParamDialogBI::~ParamDialogBI()
{
	
}

void ParamDialogBI::replaceInput(DcSignal* pitem) {

	QString blockTitle = ui.edTitle->text();
	bool isOk = m_signalManager->signalReplaced(m_ioid, pitem, blockTitle);
	if (!isOk) {
		qDebug() << "Wrong replace!";
		return;
	}

	for (auto &it : m_params) {
		if (it.index.contains("name")) it.value = blockTitle;
		else if (it.index.contains("signal")) it.value = pitem->internalId();
	}
}

void ParamDialogBI::saveChanges() {

	QModelIndex index = ui.signalsTree->currentIndex();
	if (!index.isValid()) {
		MsgBox::error("Некорректная строка выбрана в дереве сигналов");
		return;
	}

	DcSignal* signal = getItemData(index);
	if (signal == nullptr) {
		MsgBox::error("Выбранный сигнал не найден в классификаторе");
		return;
	}

	QString blockTitle = ui.edTitle->text();
	if (blockTitle.isEmpty()) {
		MsgBox::error("Имя входа алгоритма не может быть пустым");
		return;
	}

	if (m_ioid > 0) {
		replaceInput(signal);
		this->done(QDialog::Accepted);
		return;
	}

	int32_t ioid(0), algpos(0);
	bool isOk = m_signalManager->signalBusy(signal, ioid, algpos, blockTitle);
	if (!isOk) {
		return;
	}

	for (auto &it : m_params) {
		if (it.index.contains("io_id"))
			it.value = ioid;
		else if (it.index.contains("name"))
			it.value = blockTitle;
		else if (it.index.contains("signal"))
			it.value = signal->internalId();
		else if (it.index.contains("alg_pin"))
			it.value = algpos + 1; }
	this->done(QDialog::Accepted);
}

void ParamDialogBI::cancel() {
	this->done(QDialog::Rejected);
}

void ParamDialogBI::tree_selected() {
	QModelIndex index = ui.signalsTree->currentIndex();
	if (!index.isValid()) {
		return;
	}

	DcSignal* pitem = getItemData(index);
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

void ParamDialogBI::dblclicked(const QModelIndex& element) {
	saveChanges();
}

void ParamDialogBI::fillTree()
{
	formedListBySignalType_t list = m_signalManager->getSignalList(DefSignalDirection::DEF_SIG_DIRECTION_INPUT);
	QStandardItem *proot = model->invisibleRootItem();

	for (int sigTypeInd = 0; sigTypeInd < list.size(); sigTypeInd++) {
		QStandardItem* pitemSigType = new QStandardItem(list.at(sigTypeInd).first);	// Создаем элемент с Названием типа
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
			pitem->setIcon(QIcon(":icons/24/signal_in.png"));
			m_assocTable.insert(std::make_pair(pitem, curSignal));
			pitemSigType->appendRow(pitem);	}
	}

	return;
}

DcSignal * ParamDialogBI::getItemData(QModelIndex &index)
{
	QStandardItem *pitem = model->itemFromIndex(index);
	if (pitem == nullptr)
		return nullptr;
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return nullptr;

	return search->second;
}

