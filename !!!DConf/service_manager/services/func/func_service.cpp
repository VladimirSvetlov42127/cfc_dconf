#include "func_service.h"

#include "service_manager/services/func/func_service_output.h"

namespace {

const QHash<uint8_t, QString>& g_VirtualFunctionsName()
{
    static QHash<uint8_t, QString> values = {
        {VFunc::NOTUSE, "Не используется"},
        {VFunc::TEST_CNTRL, "Пуск теста цепей управления"},
        {VFunc::OSCILL_START, "Пуск осциллографирования"},
        {VFunc::VDIN_CONTROL, "Управление виртуальным входом"},
        {VFunc::XCBR_CNTRL, "Управление выключателем"},
        {VFunc::XCBR_RZA_CNTRL, "Управление выключателем c РЗА"},
        {VFunc::QUIT_CMD, "Квитация событий"},
        {VFunc::FIX_VDIN, "Фиксация входа"},
        {VFunc::VDOUT_CONFIRM, "Квитация события"},
        {VFunc::VDIN_EVENT, "Событие в виртуальном входе"},
        {VFunc::EXEC_EMBEDED_ALG, "Запуск встроенного алгоритма"},
        {VFunc::NETWUSE,	"Штатное управление"},
        {VFunc::CHANGE_SIM, "Смена СИМ карты"},
        {VFunc::BLOCK_TU, "Запретить ТУ"},
        {VFunc::CONTROL_SV, "Управление SV потоком"},
        {VFunc::ACTIVE_GROUP, "Установка активной группы уставок"}
    };

    return values;
};

} // namespace

FuncService::FuncService(const QString &name)
    : Service{ name }
{
}

Service::Type FuncService::type() const
{
    return Service::FuncVOutType;
}

void FuncService::appendVDin(VirtualInputSignal *vdin)
{
    m_vdins.emplace(vdin->subTypeID(), vdin);
}

void FuncService::appendSupportedType(uint8_t id, VFunc::Type type)
{
    m_vfuncsInfo.emplace(id, VFuncInfo{id, type, g_VirtualFunctionsName().value(type)});
}

const std::map<uint8_t, FuncService::VFuncInfo> &FuncService::supportedTypes() const
{
    return m_vfuncsInfo;
}

const VFunc::UPtrVector &FuncService::funcList() const
{
    return m_vfuncList;
}

QString FuncService::name(uint8_t id) const
{
    return supportedTypes().at(id).name;
}

VFunc::Type FuncService::type(uint8_t id) const
{
    return supportedTypes().at(id).type;
}

void FuncService::setType(uint8_t idx, uint8_t typeId)
{
    if (idx >= m_vfuncList.size())
        return;

    auto vfunc = m_vfuncList.at(idx).get();
    auto oldHasVDinArg = hasVDinArg(vfunc->typeId());
    auto newHasVDinArg = hasVDinArg(typeId);

    // меняем тип функции
    vfunc->setType(typeId);

    // удаляем выход сервиса, если у старой функции агрумент VDin, а у новой нет.
    if (oldHasVDinArg && !newHasVDinArg)
        removeOutput(vfunc);

    // добавляем выход сервиса, если у новой функции агрумент VDin, а у старой нет.
    if (!oldHasVDinArg && newHasVDinArg)
        makeOutput(vfunc);
}

void FuncService::setArgValue(uint8_t idx, uint8_t value)
{
    if (idx >= m_vfuncList.size())
        return;

    auto vfunc = m_vfuncList.at(idx).get();

    // Если для функции не создан выход сервиса, устанавливаем значение аргумента функции в value.
    auto func_output = vfunc->output();
    if (!func_output) {
        vfunc->setArgValue(value);
        return;
    }

    // Если для функции создан выход сервиса, устанавливаем этому выходу target в VDin с subTypeId = value.
    VirtualInputSignal *vdin = nullptr;
    if (auto vdinIt = m_vdins.find(value); vdinIt != m_vdins.end())
        vdin = vdinIt->second;

    func_output->setTarget(vdin);
}

VFunc *FuncService::makeVirtualFunc(VirtualOutputSignal *vdout, DcParam *typeParam, DcParam *argParam)
{
    auto vfunc = std::make_unique<VFunc>(vdout, typeParam, argParam, this);
    if (hasVDinArg(vfunc->typeId()))
        makeOutput(vfunc.get());

    return m_vfuncList.emplace_back(std::move(vfunc)).get();
}

void FuncService::makeOutput(VFunc *func)
{
    auto sio = std::make_unique<FuncServiceOutput>(func->name(), func->argParam(), this);

    // Ищем vdin у которого subTypeId равен значению аргумента функции и привязываем если он свободен (не установлен source).
    if (auto vdinIt = m_vdins.find(func->argValue()); vdinIt != m_vdins.end() ) {
        auto vdin = vdinIt->second;
        if (vdin && !vdin->source())
            sio->setTarget(vdin);
    }

    func->setOutput(sio.get());
    Service::addOutput(std::move(sio));
}

void FuncService::removeOutput(VFunc *func)
{
    auto func_output = func->output();
    if (!func_output)
        return;

    // Освободить виртуальный вход не меняя аргумента функции
    auto currentArg = func->argValue();
    func_output->setTarget(nullptr);    // сбрасывает значение аргумента функции в 0xFF.
    func->setArgValue(currentArg);
    func->setOutput(nullptr);

    Service::deleteOutput(func_output);
}

bool FuncService::hasVDinArg(uint8_t typeId) const
{
    switch (type(typeId)) {
    case VFunc::VDIN_CONTROL:
    case VFunc::VDIN_EVENT:
    case VFunc::FIX_VDIN:
        return true;
        break;
    default:
        break;
    }

    return false;
}

