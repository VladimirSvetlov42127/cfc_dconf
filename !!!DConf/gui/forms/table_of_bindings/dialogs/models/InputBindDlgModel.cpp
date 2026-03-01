#include "InputBindDlgModel.h"

InputBindDlgModel::InputBindDlgModel(QObject *parent)
	: IDlgModel(parent)
{

}

InputBindDlgModel::InputBindDlgModel(DcController *controller, QTreeView * ptree, QObject * parent)
	: IDlgModel(controller, ptree, parent)
{
}

InputBindDlgModel::~InputBindDlgModel()
{
}

void InputBindDlgModel::fillTree()
{
	formedListBySignalType_t list = m_signalManager->getSignalList(DefSignalDirection::DEF_SIG_DIRECTION_OUTPUT);

	QStandardItem *proot = m_model->invisibleRootItem();

	for (int sigTypeInd = 0; sigTypeInd < list.size(); sigTypeInd++) {

		QList<DcSignal*> signalListCurType = list.at(sigTypeInd).second;
		if (signalListCurType.isEmpty())
			continue;
		if (signalListCurType.at(0)->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)	// Нельзя привязать к виртуальным
			continue;

		QStandardItem* pitemSigType = new QStandardItem(list.at(sigTypeInd).first);	// Создаем элемент с названием типа
		QString nametmp = "(%1) %2";

		for (int signalInd = 0; signalInd < signalListCurType.size(); signalInd++) {
			DcSignal *curSignal = signalListCurType.at(signalInd);
			QString name = nametmp.arg(QString::number(curSignal->internalId()), curSignal->name());
			QStandardItem* pitem = new QStandardItem(name);
			pitem->setEditable(false);
			//pitem->setIcon(QIcon(":/images/16/output.png"));
			pitem->setIcon(QIcon(":icons/24/signal_in.png"));
			m_assocTable.insert(std::make_pair(pitem, curSignal));
			pitemSigType->appendRow(pitem);
		}
		pitemSigType->setEditable(false);
		//pitemSigType->setIcon(QIcon(":/images/16/document.png"));
		pitemSigType->setIcon(QIcon(":/icons/24/folder_project.png"));
		proot->appendRow(pitemSigType);	}
}