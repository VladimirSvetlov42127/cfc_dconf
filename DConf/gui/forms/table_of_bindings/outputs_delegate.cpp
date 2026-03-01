#include "outputs_delegate.h"


//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionViewItem>


//===================================================================================================================================================
//	Конструктор класса
//===================================================================================================================================================
OutputsDelegate::OutputsDelegate(QObject* parent) : QItemDelegate(parent)
{
	//	Свойства класса
	_current_row = -1;
}


//===================================================================================================================================================
//	Перегружаемые методы класса
//===================================================================================================================================================
void OutputsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	//	Проверка элемента
	if (!index.isValid())
		return;
	QStandardItemModel* model = (QStandardItemModel*)index.model();
	QStandardItem* item = model->itemFromIndex(index);
	if (!item->data(Qt::UserRole).isValid())
		return;

    //  Получение параметров привязки
    QString name = QString();
    OutputSignal* output = static_cast<OutputSignal*>(item->data(Qt::UserRole).value<void*>());
    if (output->source())
        name = output->source()->fullText();

    //	Вывод кнопки привязки
    QRect rectangle = option.rect;
    QStyleOptionButton button;
    button.rect = QRect(rectangle.left() + 5, rectangle.top(), 20, 20);
    name == QString() ? button.icon = QIcon(":/icons/24/connect_no.png") : button.icon = QIcon(":/icons/24/connect_yes.png");
    _current_row == index.row() ? button.state = QStyle::State_Sunken | QStyle::State_Enabled : button.state = QStyle::State_Raised | QStyle::State_Enabled;
    button.iconSize = QSize(16, 16);
    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);

    //	Опции вывода
    QStyleOptionViewItem delegate_option = option;
    delegate_option.text = name;
    delegate_option.rect = QRect(rectangle.left() + 30, rectangle.top() - 1, rectangle.width() - 10, rectangle.height());
    delegate_option.palette.setBrush(QPalette::HighlightedText, Qt::black);
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &delegate_option, painter);

	return;
}

void OutputsDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(option.rect);
}

bool OutputsDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	//	Проверка нажатия кнопки мыши
	if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease) 
		return true;

	//	Нажатие кнопки мыши
	if (event->type() == QEvent::MouseButtonPress) {
		_current_row = index.row();
		return true;
	}

	//	Отпускание кнопки мыши
	if (event->type() == QEvent::MouseButtonRelease) {
		_current_row = -1;

		//	Координаты мышки
		QMouseEvent* evnt = (QMouseEvent*) event;
		int click_x = evnt->x();
		int click_y = evnt->y();

		//	Область срабатывания мышки
		QRect rectangle = option.rect;
		int x = rectangle.left() + 5;
		int y = rectangle.top();
		if ((click_x > x && click_x < x + 25) && (click_y > y && click_y < y + 25))
            emit onClicked(index);
        return true;
	}

	return true;
}


