#include "service_manager.h"

//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QDebug>
#include <QFile>
#include <QMap>

//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include "data_model/dc_controller.h"
#include "service_manager/services/func/func_service.h"
#include "gui/forms/algorithms/custom/cfc_tools/cfc_parser.h"
#include "gui/forms/algorithms/custom/cfc_tools/cfc_parser.h"
#include "utils/bindings_update.h"
#include "file_managers/DcFlexLogicFileManager.h"

namespace {

    template<typename T>
    QList<T*> toList(const std::vector<std::unique_ptr<Service>> &services) 
    {
        QList<T*> result;
        for(auto &s: services)
            if (auto ss = dynamic_cast<T*>(s.get()); ss)
                result.append(ss);

        return result;
    }
}


//===================================================================================================================================================
//	Конструктор класса
//===================================================================================================================================================
ServiceManager::ServiceManager(DcController *controller)
    : m_controller{controller}
{
}


//===================================================================================================================================================
//	Открытые методы класса
//===================================================================================================================================================
bool ServiceManager::load(uint8_t mode)
{
    if (mode < 1)
        setBindingParams();

    return configure();
}

bool ServiceManager::configure()
{
    clear();
    return init();
}

QList<InputSignal *> ServiceManager::dins() const
{
    QList<InputSignal*> result;
    for(auto it = m_dins.begin(); it != m_dins.end(); ++it)
        result.append(it->second.get());
    return result;
}

QList<VirtualInputSignal *> ServiceManager::vdins() const
{
    QList<VirtualInputSignal*> result;
    for(auto it = m_dins.begin(); it != m_dins.end(); ++it)
        if (auto s = dynamic_cast<VirtualInputSignal*>(it->second.get()); s)
            result.append(s);
    return result;
}

QList<OutputSignal *> ServiceManager::douts() const
{
    QList<OutputSignal*> result;
    for(auto it = m_douts.begin(); it != m_douts.end(); ++it)
        result.append(it->second.get());
    return result;
}

QList<VirtualInputSignal *> ServiceManager::freeVdins() const
{
    QList<VirtualInputSignal*> result;
    for(auto it = m_dins.begin(); it != m_dins.end(); ++it)
        if (auto s = dynamic_cast<VirtualInputSignal*>(it->second.get()); s && !s->source())
            result.append(s);
    return result;
}

QList<VirtualInputSignal *> ServiceManager::busyVdins() const
{
    QList<VirtualInputSignal*> result;
    for(auto it = m_dins.begin(); it != m_dins.end(); ++it)
        if (auto s = dynamic_cast<VirtualInputSignal*>(it->second.get()); s && s->source())
            result.append(s);
    return result;
}

InputSignal *ServiceManager::din(uint16_t internalID) const
{
    if (auto it = m_dins.find(internalID); it != m_dins.end())
        return it->second.get();

    return nullptr;
}

QList<Service *> ServiceManager::services() const
{
    return toList<Service>(m_services);
}

QList<AlgService *> ServiceManager::algServices() const
{
    return toList<AlgService>(m_services);
}

QList<CfcAlgService *> ServiceManager::cfcAlgServices() const
{
    return toList<CfcAlgService>(m_services);
}

AlgService *ServiceManager::alg(uint16_t addr) const
{
    for(auto &ser: m_services) {
        auto alg = dynamic_cast<AlgService*>(ser.get());
        if (alg && alg->addr() == addr)
            return alg;
    }

    return nullptr;
}

CfcAlgService* ServiceManager::CfcAlg(uint8_t id) const
{
    for(auto &ser: m_services) {
        auto cfc_alg = dynamic_cast<CfcAlgService*>(ser.get());
        if (cfc_alg && cfc_alg->id() == id)
            return cfc_alg;
    }

    return nullptr;
}

CfcAlgService *ServiceManager::appendCfcService(const QString &name, uint8_t id, uint8_t profile)
{
    auto param_v2 = dynamic_cast<DcParamCfg_v2*>(controller()->getParam(SP_FLEXLGCROSSTABLE, 0));
    if (!param_v2 || param_v2->getProfileCount() <= profile)
        return nullptr;

    auto cfc_service_uptr = std::make_unique<CfcAlgService>(name, id);
    auto cfc_service = cfc_service_uptr.get();
    m_services.emplace_back(std::move(cfc_service_uptr));

    for(size_t i = 0; i < param_v2->getSubProfileCount(); ++i)
        cfc_service->appendPin(controller()->getParam(SP_FLEXLGCROSSTABLE, profile * PROFILE_SIZE + i));

    return cfc_service;
}

void ServiceManager::removeCfcService(uint8_t id)
{
    for (int i = 0; m_services.size(); i++) {
        auto cfc_alg = dynamic_cast<CfcAlgService*>(m_services.at(i).get());
        if (!cfc_alg)
            continue;
        if (cfc_alg->id() != id)
            continue;

        //  Отвязываем сигналы
        //  Удаляем сервис из списка
    }

    return;
}


//===================================================================================================================================================
//	Отладочный вывод
//===================================================================================================================================================
void ServiceManager::printAlgs()
{
    qDebug() << "======================================";
    qDebug() << "======================== SERVICEs";
    qDebug() << "======================================";

    for(auto &service: m_services) {
        bool isInputBinded = false;
        for(auto &input: service->inputs())
            if (input->source()) {
                isInputBinded = true;
                break;
            }

        bool isOutputBinded = false;
        for(auto &output: service->outputs())
            if (output->target()) {
                isOutputBinded = true;
                break;
            }

        if (!isInputBinded && !isOutputBinded)
            continue;

        service->print();
    }
}

void ServiceManager::printDouts()
{
    qDebug() << "======================================";
    qDebug() << "========================== DOUTs";
    qDebug() << "======================================";

    //
    // auto dinToString = [](InputSignal *in) {
    //     if (!in)
    //         return QString();
    //
    //     return QString("<--------- (%1) %2").arg(in->internalID()).arg(in->name());
    // };
    //
    // QString outTemp = QString("(%1) %2: %3");
    // for(auto &[id, dout]: m_douts) {
    //     // if (!dout->source())
    //     //     continue;
    //
    //     qDebug().noquote() << outTemp.arg(dout->internalID()).arg(dout->name(), dinToString(dout->source()));
    // }
    //

    for (size_t i = 0; i < m_douts.size(); i++) {
        auto output = m_douts.find(i)->second.get();
        qDebug() << i << output->name() << " ----> " << output->text() << "  =====> " << output->fullText();
    }
}

void ServiceManager::printDins()
{
    qDebug() << "======================================";
    qDebug() << "========================== DINs";
    qDebug() << "======================================";

    //
    // auto targetsToString = [](InputSignal *in) {
    //     if (!in)
    //         return QString();
    //
    //     QStringList list;
    //     for(auto target: in->targets()) {
    //         list.append(QString("%1").arg(target->name()));
    //     }
    //     return QString("---------> %1").arg(list.join(" | "));
    // };
    //
    // QString inTemp = QString("(%1) %2: %3");
    // for(auto &[id, din]: m_dins) {
    //     // if (din->targets().empty())
    //     //     continue;
    //
    //     qDebug().noquote() << inTemp.arg(din->internalID()).arg(din->name(), targetsToString(din.get()));
    // }
    //

    for (size_t i = 0; i < m_dins.size(); i++) {
        auto input = m_dins.find(i)->second.get();
        qDebug() << i << input->name() << " ----> " << input->text() << "=====>" << input->fullText();
    }
}


//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
void ServiceManager::setBindingParams()
{
    //  Синхронизация таблиц привязки
    BindingsUpdate* updater = new BindingsUpdate(controller());
    updater->tablesToParams();
    delete updater;

    return;
}

void ServiceManager::clear()
{
    //  Очистка параметров сервис-менеджера
    m_dins.clear();
    m_douts.clear();
    m_services.clear();

    //  Очистка таблиц привязки
    // gDbManager.beginTransaction(controller()->index());
    // for (uint32_t i = 0; i < controller()->matrix_signals()->size(); i++)
    //     controller()->matrix_signals()->get(i)->remove();
    // controller()->matrix_signals()->removeAll();

    // for (uint32_t i = 0; i < controller()->matrix_alg()->size(); i++)
    //     controller()->matrix_alg()->get(i)->remove();
    // controller()->matrix_alg()->removeAll();

    // for (uint32_t i = 0; i < controller()->matrix_cfc()->size(); i++)
    //     controller()->matrix_cfc()->get(i)->remove();
    // controller()->matrix_cfc()->removeAll();

    // //  Очистка таблиц гибкой логики
    // QString remove = "DELETE FROM alg_cfc_io;";
    // gDbManager.execute(controller()->index(), remove);
    // remove = "DELETE FROM alg_io";
    // gDbManager.execute(controller()->index(), remove);
    // gDbManager.endTransaction(controller()->index());

    return;
}

bool ServiceManager::init()
{
    //  Загрузка сигналов
    if (!loadSignals()) {
        emit errorToLog("Ошибка загрузки сигналов в менеджер сигналов.");
        return false;
    }

    //  Загрузка привязок встроенных алгоритмов
    if (!loadAlgs())
        emit warningToLog("Проблемы с загрузкой привязок встроенных алгоритмов.");

    //  Загрузка привязок алгоритмов гибкой логики
    if (!loadCfcAlgs())
        emit warningToLog("Проблемы с загрузкой привязок алгоритмов гибкой логики.");

    //  Загрузка привязок функций
    if (!loadFuncs())
        emit warningToLog("Проблемы с загрузкой привязок виртуальных функций.");

    // printAlgs();
    //printDouts();
    //printDins();

    return true;
}

bool ServiceManager::loadSignals()
{
    // Загрузка дискретных входов
    uint16_t in_subtype = 0;
    uint16_t in_ad = 0;
    uint16_t in_physical = 0;
    uint16_t in_logic = 0;
    uint16_t in_virtual = 0;
    uint16_t in_remote = 0;

    for(auto signal: m_controller->getSignalList(DEF_SIG_TYPE_DISCRETE)) {
        if (signal->subtype() == DEF_SIG_SUBTYPE_LED_AD)
            in_subtype = in_ad++;
        if (signal->subtype() == DEF_SIG_SUBTYPE_PHIS)
            in_subtype = in_physical++;
        if (signal->subtype() == DEF_SIG_SUBTYPE_LOGIC)
            in_subtype = in_logic++;
        if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)
            in_subtype = in_virtual++;
        if (signal->subtype() == DEF_SIG_SUBTYPE_REMOTE)
            in_subtype = in_remote++;

        if (DEF_SIG_SUBTYPE_VIRTUAL == signal->subtype()) {
            auto vdin = std::make_unique<VirtualInputSignal>(signal->index(),
                                                             signal->internalId(),
                                                             signal->direction(),
                                                             signal->type(),
                                                             signal->subtype(),
                                                             "Виртуальный вход " + QString::number(in_subtype),
                                                             in_subtype);
            m_dins.emplace(signal->internalId(), std::move(vdin));
        }
        else {
            auto din = std::make_unique<InputSignal>(signal->index(),
                                                     signal->internalId(),
                                                     signal->direction(),
                                                     signal->type(),
                                                     signal->subtype(),
                                                     signal->name(), in_subtype);
            m_dins.emplace(signal->internalId(), std::move(din));
        }
    }

    // Загрузка дискретных выходов
    uint16_t out_subtype = 0;
    uint16_t out_physical = 0;
    uint16_t out_logic = 0;
    uint16_t out_virtual = 0;
    uint16_t out_remote = 0;

    for(auto signal: m_controller->getSignalList(DEF_SIG_TYPE_DISCRETE, DEF_SIG_SUBTYPE_UNDEF, DEF_SIG_DIRECTION_OUTPUT)) {
        auto bindParam = m_controller->getParam(0x02F2, signal->internalId());
        if (!bindParam)
            continue;

        if (signal->subtype() == DEF_SIG_SUBTYPE_PHIS || signal->subtype() == DEF_SIG_SUBTYPE_LED_AD)
            out_subtype = out_physical++;
        if (signal->subtype() == DEF_SIG_SUBTYPE_LOGIC)
            out_subtype = out_logic++;
        if (signal->subtype() == DEF_SIG_SUBTYPE_VIRTUAL)
            out_subtype = out_virtual++;
        if (signal->subtype() == DEF_SIG_SUBTYPE_REMOTE)
            out_subtype = out_remote++;

        std::unique_ptr<OutputSignal> dout;
        if (DEF_SIG_SUBTYPE_VIRTUAL == signal->subtype()) {
            dout = std::make_unique<VirtualOutputSignal>(signal->index(),
                                                   signal->internalId(),
                                                   signal->direction(),
                                                   signal->type(),
                                                   signal->subtype(),
                                                   signal->name(),
                                                   bindParam, out_subtype);
        }
        else {
            dout = std::make_unique<OutputSignal>(signal->index(),
                                                   signal->internalId(),
                                                   signal->direction(),
                                                   signal->type(),
                                                   signal->subtype(),
                                                   signal->name(),
                                                   bindParam, out_subtype);
        }

        if (auto it = m_dins.find(bindParam->value().toUInt()); it != m_dins.end())
            dout->setSource(it->second.get());        

        m_douts.emplace(signal->internalId(), std::move(dout));
    }

    return true;
}

bool ServiceManager::loadAlgs()
{
    // Загрузка встроенных алгоритмов
    m_algId = 0;
    m_algIoId = 0;
    for (size_t i = 0; i < m_controller->algs_internal()->size(); ++i) {
        DcAlgInternal*alg = m_controller->algs_internal()->get(i);
        loadAlg(alg->property("addr").toUInt(nullptr, 16), alg->name());
    }

    loadAlg(SP_AINCMP_WORDIN_PARAM, "Сравнение Аналогов");
    loadAlg(SP_APPL_MANAGE_WORDDIN_PARAM, "Отображение группы уставок");
    loadAlg(SP_INDCON_WORDIN_PARAM, "Дискрет связи");

    return true;
}

bool ServiceManager::loadCfcAlgs()
{
    //  Установка первоначальных параметров
    DcParamCfg_v2* param_v2 = dynamic_cast<DcParamCfg_v2*>(controller()->getParam(SP_FLEXLGCROSSTABLE, 0));
    if (!param_v2) {
        emit warningToLog("Отсутствует параметр привязки гибкой логики.");
        return false;
    }


    m_cfcIoId = 0;

    //  Анализ файлов гибкой логики
    CfcParser* parser = new CfcParser();
    QString cfc_name = QString("{%1:%2}");
    for (uint16_t i = 0; i < param_v2->getProfileCount(); i++) {
        QString graph_name = DcFlexLogicFileManager(controller()).localGraphFileName(i + 1);
        if (!QFile::exists(graph_name))
            continue;
        if (!parser->loadData(graph_name))
            continue;

        //  Назначение сигналов алгоритмов гибкой логики
        QList<CfcNode*> nodes = parser->nodes();
        int nodes_count = nodes.count();
        QString title = parser->title();
        auto cfc_service = std::make_unique<CfcAlgService>(parser->title(), i);
        for(size_t p = 0; p < param_v2->getSubProfileCount(); ++p )
            cfc_service->appendPin(controller()->getParam(SP_FLEXLGCROSSTABLE, i * PROFILE_SIZE + p));

        for (int ii = 0; ii < nodes_count; ii++) {
            CfcNode* node = nodes.at(ii);
            if (node->name() != "BO" && node->name() != "BI")
                continue;

            //  Параметры входа/выхода
            int alg_pin = node->param("alg_pin").value.toInt() - 1;
            if (alg_pin < 0)
                alg_pin = cfc_service->freePin();
            uint16_t bindValue = cfc_service->bindParam(alg_pin)->value().toUInt();

            //  Вход алгоритма гибкой логики
            if (node->name() == "BI") {
                int io_id = nextCfcID();
                auto input = cfc_service->makeInput(io_id, alg_pin);
                if (auto it = m_dins.find(bindValue); it != m_dins.end())
                    input->setSource(it->second.get());
                node->setParam("io_id", io_id + 1);
                node->setParam("alg_pin", alg_pin + 1);
                bindValue == 0xFFFF ? node->setParam("signal", - 1) : node->setParam("signal", bindValue);
            }

            //  Выход алгоритма гибкой логики
            if (node->name() == "BO") {
                int io_id = nextCfcID();
                auto output = cfc_service->makeOutput(io_id, alg_pin);
                if (auto it = m_dins.find(bindValue); it != m_dins.end())
                    output->setTarget(dynamic_cast<VirtualInputSignal*>(it->second.get()));
                node->setParam("io_id", io_id + 1);
                node->setParam("alg_pin", alg_pin + 1);
                bindValue == 0xFFFF ? node->setParam("signal", - 1) : node->setParam("signal", bindValue);
            }
        }
        m_services.emplace_back(std::move(cfc_service));
        parser->saveData(graph_name);
    }
    delete parser;

    return true;
}

bool ServiceManager::loadFuncs()
{
    auto func_service = std::make_unique<FuncService>("Сервис функций виртуальных выходов");

    // Заполнение сервиса списком виртуальных входов
    for(auto vdin: vdins())
        func_service->appendVDin(vdin);

    auto vfListParam = dynamic_cast<DcParamCfg_v2*>(controller()->getParam(SP_DOUT_FUNCLIST, 0));
    if (!vfListParam)
        return false;

    auto softIdParam = controller()->getParam(SP_SOFTWARE_ID, 0);
    if (!softIdParam)
        return false;

    // Заполнение сервиса типами поддерживаемых функций
    auto softId = softIdParam->value().toUInt();
    auto isInvalidLtOscill = softId == DEPRTU_LT_T || softId == DEPRTU_LTA_T || softId == DEPRTU_LT_R || softId == DEPRTU_LTA_R ||
            softId == DEPRTU_LT_F || softId == DEPRTU_LTA_F;

    for (size_t i = 0; i < vfListParam->getSubProfileCount(); i++) {
        auto p = controller()->getParam(SP_DOUT_FUNCLIST, i);
        auto type = static_cast<VFunc::Type>(p->value().toUInt());
        if (type == VFunc::OSCILL_START && isInvalidLtOscill)
            continue;

        func_service->appendSupportedType(i, type);
    }

    //  Проверка наличия виртуальных функций
    auto param_v2 = dynamic_cast<DcParamCfg_v2*>(controller()->getParam(SP_DOUT_TYPE, 0));
    if (!param_v2) {
        emit warningToLog("Отсутствует параметр выходов виртуальных функций.");
        return false;
    }

    // Созадние функций виртуальных выходов в сервисе
    std::unordered_map<uint8_t, VirtualOutputSignal*> vdouts;
    for(auto dout: douts())
        if (auto vout = dynamic_cast<VirtualOutputSignal*>(dout); vout)
            vdouts[vout->subTypeID()] = vout;

    for (int i = 0; i < param_v2->getProfileCount(); i++) {
        DcParam* typeIdParam = controller()->getParam(SP_DOUT_TYPE, i * PROFILE_SIZE);
        DcParam* argParam = controller()->getParam(SP_DOUT_TYPE, i * PROFILE_SIZE + 1);

        auto findIt = vdouts.find(i);
        if (findIt != vdouts.end())
            func_service->makeVirtualFunc(findIt->second, typeIdParam, argParam);
    }

    m_services.emplace_back(std::move(func_service));
    return true;
}

bool ServiceManager::loadAlg(uint16_t address, const QString &name)
{
    if (!address)
        return false;

    auto alg = std::make_unique<AlgService>(++m_algId, name, address);
    auto alg_service = alg.get();
    m_services.emplace_back(std::move(alg));

    // Входы встроенных алгоритмов
    auto algInParams = dynamic_cast<DcParamCfg_v2*>(m_controller->getParam(address, 0));
    if (algInParams)
        for(uint16_t i = 0; i < algInParams->getSubProfileCount(); ++i) {
            auto inBindParam = m_controller->getParam(address, i);
            if (!inBindParam)
                continue;

            auto sin = alg_service->makeInput(++m_algIoId, inBindParam);
            if (auto it = m_dins.find(inBindParam->value().toUInt()); it != m_dins.end())
                sin->setSource(it->second.get());
        }

    // Выходы
    address++;
    auto algOutParams = dynamic_cast<DcParamCfg_v2*>(m_controller->getParam(address, 0));
    if (algOutParams)
        for(uint16_t i = 0; i < algOutParams->getSubProfileCount(); ++i) {
            auto outBindParam = m_controller->getParam(address, i);
            if (!outBindParam)
                continue;

            auto sout = alg_service->makeOutput(++m_algIoId, outBindParam);
            if (auto it = m_dins.find(outBindParam->value().toUInt()); it != m_dins.end())
                sout->setTarget(dynamic_cast<VirtualInputSignal*>(it->second.get()));
        }

    return true;
}
