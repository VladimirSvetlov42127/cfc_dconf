#include "OutputBindDlgModel.h"

OutputBindDlgModel::OutputBindDlgModel(QObject *parent)
	: IDlgModel(parent)
{
}

OutputBindDlgModel::OutputBindDlgModel(DcController *controller, QTreeView * ptree, QObject * parent)
	: IDlgModel(controller, ptree, parent)
{
}

void OutputBindDlgModel::fillTree()
{
	formedListBySignalType_t list = m_signalManager->getSignalList(DefSignalDirection::DEF_SIG_DIRECTION_INPUT);

	QStandardItem *proot = m_model->invisibleRootItem();

	for (int sigTypeInd = 0; sigTypeInd < list.size(); sigTypeInd++) {
		QStandardItem* pitemSigType = new QStandardItem(list.at(sigTypeInd).first);	// Создаем элемент с названием типа
		pitemSigType->setEditable(false);
		//pitemSigType->setIcon(QIcon(":/images/16/document.png"));
		//pitemSigType->setIcon(QIcon(":/icons/24/exchange.png"));
		pitemSigType->setIcon(QIcon(":/icons/24/folder_project.png"));
		proot->appendRow(pitemSigType);

		QString nametmp = "(%1) %2";
		QList<DcSignal*> signalListCurType = list.at(sigTypeInd).second;
		for (int signalInd = 0; signalInd < signalListCurType.size(); signalInd++) {
			DcSignal *curSignal = signalListCurType.at(signalInd);
			QString name = nametmp.arg(QString::number(curSignal->internalId()), curSignal->name());
			QStandardItem* pitem = new QStandardItem(name);
			pitem->setEditable(false);
			if (curSignal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)
				//pitem->setIcon(QIcon(":/icons/24/switches.png"));
				pitem->setIcon(QIcon(":/icons/24/signal_out.png"));
			else
				pitem->setIcon(QIcon(":/icons/24/signal_in.png"));
			m_assocTable.insert(std::make_pair(pitem, curSignal));
			pitemSigType->appendRow(pitem);	}
	}

	return;
}
