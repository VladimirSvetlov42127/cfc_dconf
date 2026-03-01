#pragma once

#include <QDialog>
#include "ui_ParamDialogBI.h"
#include <qstandarditemmodel.h>
#include <map>
#include <data_model/dc_signal.h>
#include <gui/forms/DcSignalManager.h>
#include "gui/forms/algorithms/custom/Editor/editor_node_data.h"

class ParamDialogBI : public QDialog
{
	Q_OBJECT

public:
	ParamDialogBI(QList<FlexLogic::NodeParam> &params, DcSignalManager *signalManager, QWidget *parent = Q_NULLPTR);
	~ParamDialogBI();

private:
	Ui::ParamDialogBI ui;
	QList<FlexLogic::NodeParam> &m_params;
	QStandardItemModel * model;
	std::map<QStandardItem*, DcSignal*> m_assocTable;
	int32_t m_ioid;
	DcSignalManager* m_signalManager;
	DcController* _controller;

public slots:
	void saveChanges();
	void cancel();
	void tree_selected();
	void dblclicked(const QModelIndex& element);

private:
	void fillTree();
	DcSignal* getItemData(QModelIndex &index);
	void replaceInput(DcSignal* pitem);
};
