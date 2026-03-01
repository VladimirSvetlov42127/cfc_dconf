#include "bi_node.h"

//===================================================================================================================================================
//	Подключение стандартных библиотек
//===================================================================================================================================================

//===================================================================================================================================================
//	Подключение сторонних библиотек
//===================================================================================================================================================

//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QPainterPath>
#include <QPainter>
#include <QFontMetrics>

//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include <gui/forms/algorithms/custom/flexlogic_namespace.h>


//===================================================================================================================================================
//	Конструктор и деструктор класса
//===================================================================================================================================================
BI::BI(QString ID, QSizeF node_size, QGraphicsItem* parent) : EditorNode(ID, parent)
{
    //  Настройка параметров
    SetType("BI");
    SetNodeType(FlexLogic::RZA_LOAD);
    node_size == QSizeF() ? SetNodeSize(QSizeF(150, 30)) : SetNodeSize(node_size);
    SetOutput(true);
    SetInversion(false);
    InitInputs(0, 0);
    InitSockets();
    addParam("Название", "name", QString());
    addParam("Сигнал", "signal", -1);
    addParam("io_id", "io_id", -1);
    addParam("AlgPin", "alg_pin", -1);
}

BI::~BI()
{
}


//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
void BI::paintElement(QPainter* painter)
{
    //  Формирование контура элемента
    QRectF rectangle = AvailableRect();
    QPainterPath path;
    path.moveTo(rectangle.x(), rectangle.y());
    path.lineTo(rectangle.x() + rectangle.width() - rectangle.height() / 2, rectangle.y());
    path.lineTo(rectangle.x() + rectangle.width(), rectangle.y() + rectangle.height() / 2 - FlexLogic::socket_radius);
    path.lineTo(rectangle.x() + rectangle.width(), rectangle.y() + rectangle.height() / 2 + FlexLogic::socket_radius);
    path.lineTo(rectangle.x() + rectangle.width() - rectangle.height() / 2, rectangle.y() + rectangle.height());
    path.lineTo(rectangle.x(), rectangle.y() + rectangle.height());
    path.closeSubpath();

    //  Вывод и заливка контура элемента
    painter->save();
    painter->setPen(QPen(FlexLogic::shape_color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(path, FlexLogic::shape_bkcolor);
    painter->drawPath(path);

    //  Вывод номера сигнала
    QRectF signal_rect = QRectF(rectangle.x(), rectangle.y(), rectangle.height(), rectangle.height());
    painter->drawRect(signal_rect);
    FlexLogic::NodeParam signal_param = param("signal");
    painter->setFont(FlexLogic::CHANNEL_TEXT_FONT);
    if (!signal_param.index.isEmpty() && signal_param.value.toInt() >=0) painter->drawText(signal_rect, Qt::AlignCenter, signal_param.value.toString());

    //  Вывод названия сигнала
    FlexLogic::NodeParam name_param = param("name");
    if (!name_param.index.isEmpty()) {
        QFontMetrics fm(FlexLogic::CHANNEL_TEXT_FONT);
        QRectF text_rect(QPointF(rectangle.x() + rectangle.height(), rectangle.y()), QPointF(rectangle.right() - 10, rectangle.bottom()));
		int text_width = fm.horizontalAdvance(name_param.value.toString()) + 40 + rectangle.height();
        int old_width = NodeSize().width();
		int delta_width = text_width - old_width;
        if (abs(delta_width) > 10) {
            SetNodeSize(QSizeF(text_width, NodeSize().height()));
            setPos(pos().x() - delta_width, pos().y());
			Sockets().at(0)->setPos(QPointF(NodeSize().width(), NodeSize().height() / 2));	}
        painter->setFont(FlexLogic::CHANNEL_TEXT_FONT);
        painter->drawText(text_rect, Qt::AlignCenter, name_param.value.toString()); 
        update(); }
    painter->restore();

    return;
}

