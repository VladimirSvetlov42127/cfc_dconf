#include "IMatrixDelegate.h"

#include <qevent.h>

#include "journal/JSource.h"

#include <gui/forms/table_of_bindings/dialogs/IBindDialog.h>

using namespace Dpc::Gui;


IMatrixDelegate::IMatrixDelegate(std::map<QStandardItem*, BindMatrixElem*> *mmap, DcController *controller, QObject *parent)
	: QItemDelegate(parent), m_controller(controller)
{
	m_map = mmap;
	_currRow = -1;
}

void IMatrixDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(option.rect);
}

IMatrixDelegate::~IMatrixDelegate()
{
}

void IMatrixDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStandardItemModel *pmodel = (QStandardItemModel *)index.model();
	QStandardItem *item2 = pmodel->itemFromIndex(index);
	if (item2 == nullptr)
		return;

	auto search = m_map->find(item2);
	if (search == m_map->end())
		return;
	BindMatrixElem *pobj = search->second;	// Вход, который выбран
	if (pobj == nullptr)
		return;

	if (!isSignalBusy(pobj)) {
		QStyleOptionButton button;
		QRect r = option.rect;//getting the rect of the cell 
		int x, y, w, h;
		x = r.left() + 5;//the X coordinate 
		y = r.top();//the Y coordinate 
		w = 25;//button width 
		h = 25;//button height 
		button.rect = QRect(x, y, w, h);
		//button.text = "Выбрать";
		button.icon = QIcon(":/images/16/bind.png");
		button.iconSize = QSize(16, 16);

		if (_currRow == index.row())
			button.state = QStyle::State_Sunken | QStyle::State_Enabled;
		else
			button.state = QStyle::State_Raised | QStyle::State_Enabled;
		QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
		return;
	}

	QString bindval = getBusySignalName(pobj);

	QStyleOptionButton button;
	QRect r = option.rect;//getting the rect of the cell 
	int x, y, w, h;
	x = r.left() + 5;//the X coordinate 
	y = r.top();//the Y coordinate 
	w = 25;//button width 
	h = 25;//button height 
	button.rect = QRect(x, y, w, h);
	if (_currRow == index.row())
		button.state = QStyle::State_Sunken | QStyle::State_Enabled;
	else
		button.state = QStyle::State_Raised | QStyle::State_Enabled;
	button.icon = QIcon(":/images/16/unbind.png");
	button.iconSize = QSize(16, 16);
	QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);

	QStyleOptionViewItem myOption = option;
	myOption.text = bindval;
	myOption.rect = QRect(r.left() + 30, r.top(), r.width() - 30, r.height());
	myOption.palette.setBrush(QPalette::HighlightedText, Qt::black);
	QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

bool IMatrixDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
	const QStyleOptionViewItem &option, const QModelIndex &index) {
	Q_UNUSED(model);

	if (event->type() != QEvent::MouseButtonPress &&
		event->type() != QEvent::MouseButtonRelease) {
		return true;
	}

	if (event->type() == QEvent::MouseButtonRelease) {
		_currRow = -1;
		QMouseEvent * e = (QMouseEvent *)event;
		int clickX = e->x();
		int clickY = e->y();

		QRect r = option.rect;//getting the rect of the cell 
		int x, y, w, h;
		x = r.left() + 5;//the X coordinate 
		y = r.top();//the Y coordinate 
		w = 25;//button width 
		h = 25;//button height 

		if (clickX > x && clickX < x + w)
			if (clickY > y && clickY < y + h)
				ExecuteClick(index);
	}
	else if (event->type() == QEvent::MouseButtonPress) {
		_currRow = index.row();
	}

	return true;
}


void IMatrixDelegate::ExecuteClick(const QModelIndex &index) {

	QStandardItemModel *pmodel = (QStandardItemModel *)index.model();
	QStandardItem *item2 = pmodel->itemFromIndex(index);
	if (item2 == nullptr)
		return;

	auto search = m_map->find(item2);
	if (search == m_map->end()) {
		return;
	}
	BindMatrixElem *leftSignalInModel = search->second;
	if (leftSignalInModel == nullptr)
		return;
	
	if (!isSignalBusy(leftSignalInModel)) { //bind output signal to virtual signal

		IBindDialog *dlg = createDialog(leftSignalInModel);
		if (!dlg)
			return;

		dlg->exec();
		delete dlg;
		return;
	}
	else {//erase from matrix and table

		if (!MsgBox::question("Удалить привязку?"))
			return;

		if (!removeElementBind(leftSignalInModel)) {
			gJournal.addErrorMessage(QString("Ошибка удаления связи из таблицы привязки."));
			//MsgBox::error("Ошибка удаления связи из таблицы привязки");
		}
			
	}
}