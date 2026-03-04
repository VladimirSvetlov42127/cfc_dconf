#include "bindings_update.h"


//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QDebug>
#include <QFile>

//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include "gui/forms/algorithms/custom/cfc_tools/cfc_parser.h"
#include "gui/forms/algorithms/custom/cfc_editor/cfc_node.h"
#include "file_managers/DcFlexLogicFileManager.h"


//===================================================================================================================================================
//	Конструктор класса
//===================================================================================================================================================
BindingsUpdate::BindingsUpdate(DcController* controller)
{
    _controller = controller;
}


//===================================================================================================================================================
//	Открытые методы класса
//===================================================================================================================================================
bool BindingsUpdate::tablesToParams()
{
    //  Результат выполнения функции
    bool result = true;

    //  Привязки выходов виртуальных функций
    if (!outputsToParams()) {
        emit warningToLog("Ошибка обработки выходов виртуальных функций.");
        result = false;
    }

    //  Привязки встроенных алгоритмов
    if (!algsToParams()) {
        emit warningToLog("Ошибка обработки встроенных алгоритмов.");
        result = false;
    }

    //  Привязки алгоритмов гибкой логики
    if (!cfcAlgsToParams()) {
        emit warningToLog("Ошибка обработки алгоритмов гибкой логики.");
        result = false;
    }

    return result;
}



//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
bool BindingsUpdate::outputsToParams()
{
    //  Очистка списка привязок дискретных выходов
    setParam(SP_CROSSTABLEDOUT, QString::number(0xFFFF));

    //  Заполнение списка привязок дискретных выходов
    for (uint32_t i = 0; i < controller()->matrix_signals()->size(); i++) {
        DcMatrixElementSignal *matrix = controller()->matrix_signals()->get(i);
        DcSignal *input = controller()->getSignal(matrix->src());
        DcSignal *output = controller()->getSignal(matrix->dst());
        if (!input || !output)
            continue;

        DcParam *param = controller()->getParam(SP_CROSSTABLEDOUT, output->internalId());
        param->updateValue(QString::number(input->internalId()));
    }

    return true;
}

bool BindingsUpdate::algsToParams()
{
    //  Очистка списка привязок встроенных алгоритмов
    for (uint32_t i = 0; i < controller()->algs_internal()->size(); i++) {
        DcAlgInternal *alg = controller()->algs_internal()->get(i);
        uint32_t address = alg->property("addr").toUInt(nullptr, 16);
        if (!address)
            continue;

        setParam(address, QString::number(0xFFFF));
        setParam(address + 1, QString::number(0xFFFF));
    }

    //  Заполнение списка привязок встроенных алгоритмов
    for (uint32_t i = 0; i < controller()->matrix_alg()->size(); i++) {
        DcMatrixElementAlg* element = controller()->matrix_alg()->get(i);
        if (!element)
            continue;

        DcAlgInternal *internal_alg = getAlgByIO(controller(), element->src());
        if (internal_alg == nullptr)
            return false;

        DcParamCfg *param = nullptr;
        DcAlgIOInternal *pios = internal_alg->ios()->getById(element->src());
        uint32_t address = internal_alg->property("addr").toUInt(nullptr, 16);
        if (!address) {
            int32_t pos = internal_alg->position() * PROFILE_SIZE + pios->pin();
            param = controller()->params_cfg()->get(SP_CROSSTABLE, pos);
        }
        else
            param = controller()->params_cfg()->get(address + pios->direction() - 1, pios->pin());

        if (!param)
            continue;

        DcSignal *psignal = controller()->getSignal(element->dst());
        if (!psignal)
            continue;

        param->updateValue(QString::number(psignal->internalId()));
    }

    return true;
}

bool BindingsUpdate::cfcAlgsToParams()
{
    //  Проверка наличия параметров гибкой логики
    DcParamCfg_v2* param_v2 = dynamic_cast<DcParamCfg_v2*>(controller()->getParam(SP_FLEXLGCROSSTABLE, 0));
    if (!param_v2)
        return false;

    //  Очистка списка привязок гибкой логики
    setParam(SP_FLEXLGCROSSTABLE, QString::number(0xFFFF));

     //  Подготовка к работе с гибкой логикой
    CfcParser parser;

    //  Анализ файлов гибкой логики
    for (uint16_t i = 0; i < param_v2->getProfileCount(); i++) {
        QString graph_name = DcFlexLogicFileManager(controller()).localGraphFileName(i + 1);
        if (!QFile::exists(graph_name))
            continue;
        if (!parser.loadData(graph_name))
            continue;

        //  Назначение сигналов алгоритмов гибкой логики
        QList<CfcNode*> nodes = parser.nodes();
        int nodes_count = nodes.count();
        for (int ii = 0; ii < nodes_count; ii++) {
            CfcNode* node = nodes.at(ii);
            if (node->nodeType() != RZA_LOAD)
                continue;
            int signal_id = node->param("signal").value.toInt();
            int pin = node->param("alg_pin").value.toInt() - 1;
            if (signal_id < 0 || pin < 0)
                continue;
            int global_io = node->param("io_id").value.toInt();
            if (node->name() == "BO") signal_id = getVirtualID(i, global_io);
            DcParam *param = controller()->getParam(SP_FLEXLGCROSSTABLE, i * PROFILE_SIZE + pin);
            param->updateValue(QString::number(signal_id));
        }
    }

    return true;
}


//===================================================================================================================================================
//	Вспомогательные утилиты
//===================================================================================================================================================
void BindingsUpdate::setParam(uint16_t address, const QString& data)
{
    DcParamCfg_v2* param_v2 = dynamic_cast<DcParamCfg_v2*>(controller()->getParam(address, 0));
    if (!param_v2)
        return;

    for (uint16_t i = 0; i < param_v2->getProfileCount(); i++) {
        for (uint16_t ii = 0; ii < param_v2->getSubProfileCount(); ii++) {
            DcParam* param = controller()->getParam(address, i * PROFILE_SIZE + ii);
            param->updateValue(data);
        }
    }

    return;
}

DcAlgInternal* BindingsUpdate::getAlgByIO(DcController *controller, int32_t source_id)
{
    for (uint32_t i = 0; i < controller->algs_internal()->size(); i++) {
        auto palg = controller->algs_internal()->get(i);
        for (uint32_t j = 0; j < palg->ios()->size(); j++) {
            auto pcfcio = palg->ios()->get(j);
            if (pcfcio->index() == source_id)
                return palg;
        }
    }

    return nullptr;
}

uint16_t BindingsUpdate::getVirtualID(int index, int global_io)
{
    auto cfc = controller()->algs_cfc()->get(index);
    DcAlgIOCfc *pio = cfc->ios()->getById(global_io);
    if (!pio)
        return 0xFFFF;

    DcMatrixElementAlgCfc *pmatrix = controller()->matrix_cfc()->get(pio->index());
    if (!pmatrix)
        return 0xFFFF;

    return controller()->getSignal(pmatrix->dst())->internalId();
}
