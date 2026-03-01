#pragma once

#include <map>
#include <qtreeview.h>
#include <qitemselectionmodel.h>
#include <qstandarditemmodel.h>

#include <data_model/dc_controller.h>

#include <gui/forms/DcSignalManager.h>

class IDlgModel : public QObject
{
	Q_OBJECT

public:
	IDlgModel(QObject *parent);
	IDlgModel(DcController *controller, QTreeView *ptree, QObject *parent);
	virtual ~IDlgModel();

	void clear();
	virtual void fillTree() = 0;

	//	Вывод сигналов виртуальных функций
	DcSignal* getItemData(QModelIndex &index);

protected:
	QStandardItemModel * m_model;
	QTreeView *m_tree;
	QObject *m_parent;
	std::map<QStandardItem*, DcSignal*> m_assocTable;
	QItemSelectionModel *m_selectionModel;
	DcController *m_controller;
	DcSignalManager *m_signalManager;

signals:
	void signal_newSignalChoosed(DcSignal *choosenSignal);

public slots:
	void slot_itemSelectedChanged(const QModelIndex &current, const QModelIndex &previous);
};
