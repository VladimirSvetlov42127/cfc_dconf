#include "bo_node.h"


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

//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include <gui/forms/algorithms/custom/flexlogic_namespace.h>


//===================================================================================================================================================
//	Конструктор и деструктор класса
//===================================================================================================================================================
BO::BO(QString ID, QSizeF node_size, QGraphicsItem* parent) : EditorNode(ID, parent)
{
    //  Настройка параметров
    SetType("BO");
    SetNodeType(FlexLogic::RZA_LOAD);
    node_size == QSizeF() ? SetNodeSize(QSizeF(150, 30)) : SetNodeSize(node_size);
    SetOutput(false);
    SetInversion(false);
    InitInputs(1, 1);
    InitSockets();
    addParam("Название", "name", QString());
    addParam("Сигнал", "signal", -1);
    addParam("io_id", "io_id", -1);
    addParam("AlgPin", "alg_pin", -1);
}

BO::~BO()
{
}


//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
void BO::paintElement(QPainter* painter)
{
    //  Формирование контура элемента
    QRectF rectangle = AvailableRect();
    QPainterPath path;
    path.moveTo(rectangle.x() + rectangle.width(), rectangle.y());
    path.lineTo(rectangle.x() + rectangle.width(), rectangle.y() + rectangle.height());
    path.lineTo(rectangle.x() + rectangle.height() / 2, rectangle.y() + rectangle.height());
    path.lineTo(rectangle.x(), rectangle.y() + rectangle.height() / 2 + FlexLogic::socket_radius);
    path.lineTo(rectangle.x(), rectangle.y() + rectangle.height() / 2 - FlexLogic::socket_radius);
    path.lineTo(rectangle.x() + +rectangle.height() / 2, rectangle.y());
    path.closeSubpath();

    //  Вывод и заливка контура элемента
    painter->save();
    painter->setPen(QPen(FlexLogic::shape_color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(path, FlexLogic::shape_bkcolor);
    painter->drawPath(path);

    //  Вывод номера сигнала
    QRectF signal_rect = QRectF(rectangle.width() - rectangle.height(), rectangle.y(), rectangle.height(), rectangle.height());
    painter->drawRect(signal_rect);
    FlexLogic::NodeParam signal_param = param("signal");
    painter->setFont(FlexLogic::CHANNEL_TEXT_FONT);
    if (!signal_param.index.isEmpty() && signal_param.value.toInt() >= 0) painter->drawText(signal_rect, Qt::AlignCenter, signal_param.value.toString());

    //  Вывод параметров элемента
    FlexLogic::NodeParam node_param = param("name");
    if (!node_param.index.isEmpty()) {
        QFontMetrics fm(FlexLogic::CHANNEL_TEXT_FONT);
        QRectF text_rect(QPointF(rectangle.left() + 10, rectangle.top()), QPointF(rectangle.right() - rectangle.height(), rectangle.bottom()));
        int text_width = fm.horizontalAdvance(node_param.value.toString()) + 40 + rectangle.height();
        int old_width = NodeSize().width();
        int delta_width = text_width - old_width;
        if (abs(delta_width) > 10) SetNodeSize(QSizeF(text_width, NodeSize().height()));
        painter->drawText(text_rect, Qt::AlignCenter, node_param.value.toString());
        update(); }
    painter->restore();

    return;
}
