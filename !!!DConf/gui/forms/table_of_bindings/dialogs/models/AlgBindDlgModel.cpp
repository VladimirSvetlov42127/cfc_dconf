#include "AlgBindDlgModel.h"

AlgBindDlgModel::AlgBindDlgModel(DcController *controller, DcAlgIO* alg, QTreeView * ptree, QObject * parent)
	: IDlgModel(controller, ptree, parent), m_alg(alg)
{
}

void AlgBindDlgModel::fillTree()
{
	QStandardItem *proot = m_model->invisibleRootItem();
	formedListBySignalType_t list;
	
	if (m_alg->direction() == IO_DIRECTION_OUTPUT) {
		
		list = m_signalManager->getSignalList(DefSignalDirection::DEF_SIG_DIRECTION_OUTPUT);

		for (int sigTypeInd = 0; sigTypeInd < list.size(); sigTypeInd++) {
			QString signalTypeName = list.at(sigTypeInd).first;
			QStandardItem* pitemSigType = new QStandardItem(signalTypeName);
			pitemSigType->setEditable(false);
			//pitemSigType->setIcon(QIcon(":/images/16/documents.png"));
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
				//pitem->setIcon(QIcon(":/images/16/document.png"));
				//pitem->setIcon(QIcon(":/icons/24/folder_project.png"));
				pitem->setIcon(QIcon(":icons/24/signal_out.png"));
				m_assocTable.insert(std::make_pair(pitem, curSignal));
				pitemSigType->appendRow(pitem);
			}
		}
	}
	else {

		list = m_signalManager->getSignalList(DefSignalDirection::DEF_SIG_DIRECTION_INPUT);

		for (int sigTypeInd = 0; sigTypeInd < list.size(); sigTypeInd++) {
			QString signalTypeName = list.at(sigTypeInd).first;
			QStandardItem* pitemSigType = new QStandardItem(signalTypeName);
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
				//pitem->setIcon(QIcon(":/images/16/document.png"));
				//pitem->setIcon(QIcon(":/icons/24/folder_project.png"));
				pitem->setIcon(QIcon(":icons/24/signal_in.png"));
				m_assocTable.insert(std::make_pair(pitem, curSignal));
				pitemSigType->appendRow(pitem);
			}
		}
	}
}