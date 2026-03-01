#pragma once

#include <QDialog>
#include "ui_ParamDialogBO.h"
#include "gui/forms/algorithms/custom/Editor/editor_node_data.h"
#include <qstandarditemmodel.h>
#include <map>
#include <data_model/dc_signal.h>
#include <gui/forms/DcSignalManager.h>

class ParamDialogBO : public QDialog
{
	Q_OBJECT

public:
	ParamDialogBO(QList<FlexLogic::NodeParam> &params, DcSignalManager *signalManager, QWidget *parent = Q_NULLPTR);
	~ParamDialogBO();

private:
	Ui::ParamDialogBO ui;
	QList<FlexLogic::NodeParam> &m_params; // Каждый выход содержит параметры
	QStandardItemModel * model;
	std::map<QStandardItem*, DcSignal*> m_assocTableTree;
	int32_t m_ioid;	// Внутренний номер выхода в разделе алгоритмов
	int32_t m_virt_dout_ind = 0;	// Сохраняем индекс виртуального входа, чтобы по нему определить старый физический выход
	int32_t m_virtInternalId;
	DcSignalManager *m_signalManager;
	DcController* _controller;

public slots:
	void onSaveChanges();
	void onCancel();
	void onTreeSelection();
	void dblclicked(const QModelIndex& element);
	
private:
	void fillTree();
	DcSignal* getItemDataTree(QModelIndex &index);
	void replaceOut();
	void createNewDout();
};
