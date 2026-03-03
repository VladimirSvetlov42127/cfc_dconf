#include "cfc_basic_scene.h"


//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include <gui/forms/algorithms/custom/cfc_nodes/cfc_nodes_list.h>


//===================================================================================================================================================
//	список переменных
//===================================================================================================================================================
namespace {
    QColor scene_bkcolor = QColor(255,255,255);
}


//===================================================================================================================================================
//	Конструктор класса
//===================================================================================================================================================
CfcBasicScene::CfcBasicScene(uint16_t id, ServiceManager* service_manager, const QString& title, QGraphicsScene* parent) : QGraphicsScene(parent)
{
    _menu_point = QPointF();
    _basic_point = QPointF();
    _grid_enable = true;
    _service_manager = service_manager;
    _cfc_service = _service_manager->CfcAlg(id);
    if (_cfc_service)
        _service_manager->appendCfcService(QString(), id, id);

    //  Вывод заголовка
    _title_item = new CfcTitle(title);
    addItem(_title_item);
    _title_item->setPos((sceneRect().width() + _title_item->width()) / 2 + 150, 0);

    setItemIndexMethod(QGraphicsScene::NoIndex);
    setBackgroundBrush(scene_bkcolor);
}


//===================================================================================================================================================
//	Открытые методы класса
//===================================================================================================================================================
CfcNode* CfcBasicScene::newEditorNode(QString name)
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

QList<CfcNode*> CfcBasicScene::nodes() const
{
    QList<CfcNode*> list;
    int count = items().count();
    for (int i = 0; i < count; i++) {
        CfcNode* node = dynamic_cast<CfcNode*>(items().at(i));
        if (!node)
            continue;
        list.append(node);
    }

    return list;
}

QList<CfcLink*> CfcBasicScene::links() const
{
    QList<CfcLink*> list;
    int count = items().count();
    for (int i = 0; i < count; i++) {
        CfcLink* link = dynamic_cast<CfcLink*>(items().at(i));
        if (!link)
            continue;
        list.append(link);
    }

    return list;
}

QList<CfcNode*> CfcBasicScene::selectedNodes() const
{
    QList<CfcNode*> list;
    int count = selectedItems().count();
    for (int i = 0; i < count; i++) {
        CfcNode* node = dynamic_cast<CfcNode*>(selectedItems().at(i));
        if (!node)
            continue;
        list.append(node);
    }

    return list;
}

QList<CfcLink*> CfcBasicScene::selectedLinks() const
{
    QList<CfcLink*> list;
    int count = selectedItems().count();
    for (int i = 0; i < count; i++) {
        CfcLink* link = dynamic_cast<CfcLink*>(selectedItems().at(i));
        if (!link)
            continue;
        list.append(link);
    }

    return list;
}

bool CfcBasicScene::dataProcessing(QList<CfcNode*> nodes, QList<CfcLink*> links)
{
    nodesProcessing(nodes);
    return validate(nodes, links);
}

bool CfcBasicScene::validate(QList<CfcNode*> nodes, QList<CfcLink*> links)
{
    bool error = true;

    //  Проверка состояния элементов BI/BO
    QString error_text_1 = QString("Элемент %1 ID= %2. - Отстутствие сервиса.");
    QString error_text_2 = QString("Элемент %1 ID= %2. - Не привязан сигнал.");
    for (int i = 0; i < nodes.count(); i++) {
        if (nodes.at(i)->name() != "BI" && nodes.at(i)->name() != "BO")
            continue;

        if (nodes.at(i)->name() == "BI") {
            CfcBI* bi_node = static_cast<CfcBI*>(nodes.at(i));
            if (!bi_node->cfcInput()) {
                error = false;
                QString message = error_text_1.arg(bi_node->name(), bi_node->id());
                emit warningToLog(message);
            }
            if (bi_node->cfcInput() && !bi_node->cfcInput()->source()) {
                error = false;
                QString message = error_text_2.arg(bi_node->name(), bi_node->id());
                emit warningToLog(message);
            }
            continue;
        }

        if (nodes.at(i)->name() == "BO") {
            CfcBO* bo_node = static_cast<CfcBO*>(nodes.at(i));
            if (!bo_node->cfcOutput()) {
                error = false;
                QString message = error_text_1.arg(bo_node->name(), bo_node->id());
                emit warningToLog(message);
            }
            if (bo_node->cfcOutput() &&  !bo_node->cfcOutput()->target()) {
                error = false;
                QString message = error_text_2.arg(bo_node->name(), bo_node->id());
                emit warningToLog(message);
            }
            continue;
        }
    }

    //  Перебор элементов для проверки наличия соединений
    QString error_text = QString("Элемент %1 ID= %2. Сокет[%3] - Отстутствие соединения.");
    for (int i = 0; i < nodes.count(); i++) {
        CfcNode* node = nodes.at(i);
        QString node_name = node->name();
        QString node_id = node->id();

        if (node_name == "BI" || node_name == "BO")
            continue;

        //  Проверка сокетов
        for (uint8_t ii = 0; ii < node->sockets().count(); ii++) {
            CfcSocket* socket = node->sockets().at(ii);
            if (socket->links().isEmpty()) {
                error = false;
                QString message = error_text.arg(node_name, node_id, QString::number(ii));
                emit warningToLog(message);
            }
        }
    }

    //  Проверка соединений
    QString link_source = QString("Связь ID= %1 Отсутствие источника сигнала.");
    QString link_target = QString("Связь ID= %1 Отсутствие приемника сигнала.");
    for (int i = 0; i < links.count(); i++) {
        CfcLink* link = links.at(i);
        if (!link->source()) {
            error = false;
            QString message = link_source.arg(link->id());
            emit warningToLog(message);
        }
        if (!link->target()) {
            error = false;
            QString message = link_target.arg(link->id());
            emit warningToLog(message);
        }
    }

    return error;
}


//===================================================================================================================================================
//	Методы обработки сигналов сцены
//===================================================================================================================================================
void CfcBasicScene::removeSelected()
{
    //  Удаление выделенных связей
    QList<CfcLink*>cfc_links = selectedLinks();
    for (int i =0; i <cfc_links.count(); i++)
        removeLink(cfc_links.at(i));

    //  Удаление выделенных узлов
    QList<CfcNode*>cfc_nodes = selectedNodes();
    for (int i =0; i <cfc_nodes.count(); i++)
        removeNode(cfc_nodes.at(i));

    return;
}

void CfcBasicScene::copySelected()
{
    QList<CfcNode*> nodes = selectedNodes();
    for (int i = 0; i < nodes.count(); i++) {
        CfcNode* node = copyNode(nodes.at(i));
        _buffer_nodes.append(node);
    }

    _basic_point = menuPoint();

    return;
}

void CfcBasicScene::pasteSelected()
{
    QPointF delta = menuPoint() - _basic_point;

    //  Вывод узлов
    for (int i = 0; i < _buffer_nodes.count(); i++) {
        CfcNode* node = _buffer_nodes.at(i);
        node->setPos(node->pos() + delta);
        addItem(node);
    }
    _buffer_nodes.clear();

    return;
}

void CfcBasicScene::cutSelected()
{
    copySelected();
    removeSelected();
    return;
}


//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
CfcNode* CfcBasicScene::nodeByID(const QString& id)
{
    QList<CfcNode*> cfc_nodes = nodes();

    for (int i = 0; i < cfc_nodes.count(); i++)
        if (cfc_nodes.at(i)->id() == id)
            return cfc_nodes.at(i);

    return nullptr;
}

CfcLink* CfcBasicScene::linkByID(const QString& id)
{
    QList<CfcLink*> cfc_links = links();

    for (int i = 0; i < cfc_links.count(); i++)
        if (cfc_links.at(i)->id() == id)
            return cfc_links.at(i);

    return nullptr;
}

void CfcBasicScene::nodesProcessing(QList<CfcNode*> nodes)
{
    //  Привязка BI/BO к соответствующим сервисам
    for (int i = 0; i < nodes.count(); i++) {

        if (nodes.at(i)->name() != "BI" && nodes.at(i)->name() != "BO")
            continue;

        if (nodes.at(i)->name() == "BI") {
            CfcBI* bi_node = dynamic_cast<CfcBI*>(nodes.at(i));
            uint8_t pin = bi_node->pin();
            if (pin < 0)
                continue;
            CfcServiceInput* input = cfcService()->input(pin);
            bi_node->setCfcInput(input);
            continue;
        }

        if (nodes.at(i)->name() == "BO") {
            CfcBO* bo_node = dynamic_cast<CfcBO*>(nodes.at(i));
            uint8_t pin = bo_node->pin();
            if (pin < 0)
                continue;
            CfcServiceOutput* output = cfcService()->output(pin);
            bo_node->setCfcOutput(output);
            continue;
        }
    }

    return;
}

void CfcBasicScene::removeLink(CfcLink* link)
{
    //  Удаление источника чигнала
    QString source_id = link->sourceID();
    uint8_t source_index = link->sourceIndex();
    CfcNode* source_node =nodeByID(source_id);
    source_node->sockets().at(source_index)->removeLink(link);

    //  Отвязывание сигнала
    if (source_node->name() == "BI"){
        CfcBI* bi_node = static_cast<CfcBI*>(source_node);
        bi_node->cfcInput()->setSource(nullptr);
        bi_node->setParam("signal", -1);
        bi_node->setParam("name", QString());
    }

    //  Удаление приемника чигнала
    QString target_id = link->targetID();
    uint8_t target_index = link->targetIndex();
    CfcNode* target_node =nodeByID(target_id);
    target_node->sockets().at(target_index)->removeLink(link);

    //  Отвязывание сигнала
    if (target_node->name() == "BO"){
        CfcBO* bo_node = static_cast<CfcBO*>(target_node);
        bo_node->cfcOutput()->setTarget(nullptr);
        bo_node->setParam("signal", -1);
        bo_node->setParam("name", QString());
    }
    delete link;
    update();
}

void CfcBasicScene::removeNode(CfcNode* node)
{
    //  Удаление элементов BI/BO
    if (node->name() == "BI") {
        CfcBI* bi_node = static_cast<CfcBI*>(node);
        if (bi_node->cfcInput()) {
            if (bi_node->cfcInput()->source())
                bi_node->cfcInput()->setSource(nullptr);
            cfcService()->removeInput(bi_node->cfcInput());
        }
        CfcSocket* socket= node->sockets().at(0);
        for (int i = 0; i < socket->links().count(); i++)
            delete socket->links().at(i);
        delete node;
        return;
    }

    if (node->name() == "BO") {
        CfcBO* bo_node = static_cast<CfcBO*>(node);
        if (bo_node->cfcOutput()) {
            if (bo_node->cfcOutput()->target())
                bo_node->cfcOutput()->setTarget(nullptr);
            cfcService()->removeOutput(bo_node->cfcOutput());
        }
        CfcSocket* socket= node->sockets().at(0);
        for (int i = 0; i < socket->links().count(); i++)
            delete socket->links().at(i);
        delete node;
        return;
    }

    //  Удаление остальных элементов
    for (int i = 0; i < node->sockets().count(); i++) {
        CfcSocket* socket= node->sockets().at(i);
        for (int ii = 0; ii < socket->links().count(); ii++)
            delete socket->links().at(ii);
    }
    delete node;

    return;
}

CfcNode* CfcBasicScene::copyNode(CfcNode* source)
{
    CfcNode* node = newEditorNode(source->name());
    if (!node)
        return nullptr;

    //  Основные параметры
    node->setSize(source->size());
    node->setPos(source->pos());

    //  Обработка BI/BO
    if (node->name() == "BI" || node->name() == "BO") {
        node->setParam("signal", -1);
        return node;
    }

    //  Добавление входов
    if (source->inputs() != node->inputs())
        node->setInputs(source->inputs());

    //  Обновление параметров
    for (int i = 0; i < source->paramsList().count(); i++)
        node->setParam(source->paramsList().at(i).index, source->paramsList().at(i).value);

    return node;
}





