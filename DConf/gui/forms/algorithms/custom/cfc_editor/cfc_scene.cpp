#include "cfc_scene.h"


//===================================================================================================================================================
//	Подключение сторонних библиотек
//===================================================================================================================================================
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QDebug>

#include <QColor>
#include <QMimeData>
#include <QByteArray>
#include <QSizeF>


//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include <gui/forms/algorithms/custom/cfc_nodes/cfc_nodes_list.h>
#include "service_manager/services/alg_cfc/cfc_service_input.h"
#include "service_manager/services/alg_cfc/cfc_service_output.h"
#include "gui/forms/algorithms/custom/cfc_editor/cfc_socket.h"


//===================================================================================================================================================
//	список переменных
//===================================================================================================================================================
namespace {
    uint8_t grid_step = 50;
    uint8_t grid_small_step = 10;
    QColor grid_color = QColor(200, 200, 255, 125);
}


//===================================================================================================================================================
//	Конструктор класса
//===================================================================================================================================================
CfcScene::CfcScene(uint16_t id, ServiceManager* service_manager, const QString& title, QGraphicsScene* parent) : CfcBasicScene(id, service_manager, title, parent)
{
    _new_link = nullptr;
}


//===================================================================================================================================================
//	Открытые методы класса
//===================================================================================================================================================
void CfcScene::showItems(const QList<CfcNode*>& nodes, const QList<CfcLink*>& links)
{
    if (!dataProcessing(nodes, links))
        Dpc::Gui::MsgBox::warning("Схема содержит ошибки.");

    for (int i = 0; i < nodes.count(); i++)
        addItem(nodes.at(i));
    for (int i = 0; i < links.count(); i++)
        addItem(links.at(i));

    return;
}

CfcNode* CfcScene::newEditorNode(QString name)
{
    if (name == "And")
        return new CfcAnd();
    if (name == "Generator")
        return new CfcGenerator();
    if (name == "ImpulsePF")
        return new CfcImpulsePF();
    if (name == "ImpulseZF")
        return new CfcImpulseZF();
    if (name == "Not")
        return new CfcNot();
    if (name == "Or")
        return new CfcOr();
    if (name == "ReturnDelay")
        return new CfcReturnDelay();
    if (name == "RsTrigger")
        return new CfcRsTrigger();
    if (name == "TriggerDelay")
        return new CfcTriggerDelay();
    if (name == "Xor")
        return new CfcXor();

    if (name == "BI") {
        CfcBI* node = new CfcBI();
        CfcServiceInput* input = cfcService()->makeInput(serviceManager()->nextCfcID(), cfcService()->freePin());
        node->setCfcInput(input);
        return node;
    }

    if (name == "BO") {
        CfcBO* node = new CfcBO();
        CfcServiceOutput* output = cfcService()->makeOutput(serviceManager()->nextCfcID(), cfcService()->freePin());
        node->setCfcOutput(output);
        return node;
    }

    return nullptr;
}


//===================================================================================================================================================
//	Перегружаемые методы класса
//===================================================================================================================================================
void CfcScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (!gridEnabled()) return;

    //  Определение переменных
    int left = int(sceneRect().left());
    int right = int(rect.right());
    int top = int(sceneRect().top());
    int bottom = int(rect.bottom());

    int first_top = top - top % grid_step;
    int first_left = left - left % grid_step;

    //  Крупная сетка
    painter->setPen(QPen(grid_color, 2));
    for (int x = first_left; x <= right; x = x + grid_step) painter->drawLine(x, top, x, bottom);
    for (int y = first_top; y <= bottom; y = y + grid_step) painter->drawLine(left, y, right, y);

    //  Мелкая сетка
    painter->setPen(QPen(grid_color, 1));
    for (int y = first_top; y <= bottom; y = y + grid_small_step) painter->drawLine(left, y, right, y);
    for (int x = first_left; x <= right; x = x + grid_small_step) painter->drawLine(x, top, x, bottom);

    return;
}

void CfcScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
}

void CfcScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
}

void CfcScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    if (!mime->hasFormat("Node")) {
        event->ignore();
        return;	}

    QByteArray data = mime->data("Node");
    QString idx = QString(data);
    clearSelection();

    CfcNode* node = newEditorNode(idx);
    if (!node) {
        Dpc::Gui::MsgBox::error(QString("Ошибка создания графического элемента %1").arg(idx));
        return;	}

    QSizeF size = node->size();
    QPointF position = QPointF(event->scenePos().x() - size.width() / 2, event->scenePos().y() - size.height() / 2);
    node->setPos(position);
    node->setSelected(false);
    addItem(node);

    return;
}

void CfcScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF position = QPointF(event->scenePos().x(), event->scenePos().y());
    CfcSocket* socket = dynamic_cast<CfcSocket*>(itemAt(position, QTransform()));

    //  Начало рисования нового соединения
    if (event->button() == Qt::LeftButton && socket) {

        //  Проверка входного сокета
        if (socket->socketType() == CfcSocket::INPUT_SOCKET && socket->links().count() > 0) {
            Dpc::Gui::MsgBox::error(QString("Данный вход уже используется."));
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        //  Добавление новой связи
        clearSelection();
        _new_link = new CfcNewLink(socket/*, position*/);
        addItem(_new_link);


        //QGraphicsScene::mousePressEvent(event);
        event->setAccepted(true);

        return;
    }
    QGraphicsScene::mousePressEvent(event);
    //event->setAccepted(true);
}

void CfcScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    //  Рисование нового соединения
    if (_new_link && (event->buttons() & Qt::LeftButton)) {
        //qDebug() << "scene draw";
        _new_link->mouseMoveEvent(event);
        //event->ignore();
        return;
    }
}

void CfcScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    //  Завершение отрисовки новой связи между элементами
    if (_new_link && event->button() == Qt::LeftButton) {
        _new_link->mouseReleaseEvent(event);

        if (_new_link->source() && _new_link->target()) {
            CfcLink* link = new CfcLink(QString(), _new_link->points());
            link->setSource(_new_link->source());
            link->setTarget(_new_link->target());
            addItem(link);
        }

        removeItem(_new_link);
        delete _new_link;
        _new_link = nullptr;
        event->accept();
    }
    QGraphicsScene::mouseReleaseEvent(event);

}




//===================================================================================================================================================
//	Методы обработки сигналов сцены
//===================================================================================================================================================


//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================






