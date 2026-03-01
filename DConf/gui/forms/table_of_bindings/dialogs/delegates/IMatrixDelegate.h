#pragma once

#include <map>
#include <qitemdelegate.h>

#include <data_model/dc_controller.h>

#include <dpc/gui/dialogs/msg_box/MsgBox.h>

class QStandardItem;

class IBindDialog;

class IMatrixDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	IMatrixDelegate(std::map<QStandardItem*, BindMatrixElem*> *mmap, DcController *controller, QObject *parent = 0);
	virtual ~IMatrixDelegate();

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
	std::map<QStandardItem*, BindMatrixElem*> *m_map;
	int _currRow;
	DcController *m_controller;

protected:
	virtual QString getBusySignalName(BindMatrixElem* element) const = 0;
	virtual bool isSignalBusy(BindMatrixElem* element) const = 0;
	virtual bool removeElementBind(BindMatrixElem* element) = 0;
	virtual IBindDialog* createDialog(BindMatrixElem* element) = 0;

private:
	void ExecuteClick(const QModelIndex &index);
};

