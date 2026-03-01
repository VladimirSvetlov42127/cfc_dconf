#include "vfunc.h"

#include "data_model/storage_for_controller_params/dc_param.h"
#include "service_manager/services/func/func_service.h"
#include "service_manager/services/func/func_service_output.h"

VFunc::VFunc(VirtualOutputSignal *vdout, DcParam *typeParam, DcParam *argParam, FuncService *service)
    : m_vdout{ vdout }
    , m_typeParam{ typeParam }
    , m_argParam{ argParam}
    , m_service{ service }
    , m_output{ nullptr }
{
    m_vdout->setFunc(this);
}

QString VFunc::name() const
{
    return service()->name(typeId());
}

QString VFunc::text() const
{
    QString tempText = QString("%1<%2>");
    switch (type()) {
    case VFunc::NOTUSE:
    case VFunc::TEST_CNTRL:
    case VFunc::OSCILL_START:
    case VFunc::QUIT_CMD:
    case VFunc::EXEC_EMBEDED_ALG:
    case VFunc::NETWUSE:
    case VFunc::CHANGE_SIM:
    case VFunc::BLOCK_TU:
        return name();
    case VFunc::VDIN_CONTROL:
    case VFunc::VDIN_EVENT:
    case VFunc::FIX_VDIN:
        return tempText.arg(name(), output() && output()->target() ? QString::number(output()->target()->internalID()) : QString());
    case VFunc::XCBR_CNTRL:
    case VFunc::ACTIVE_GROUP:
        return tempText.arg(name()).arg(argValue() + 1);
    case VFunc::CONTROL_SV:
        return tempText.arg(name(), argValue() == 0 ? "Откл" : argValue() == 1 ? "Вкл" : QString());
    default:
        return tempText.arg(name()).arg(argValue());
    }

    return QString();
}

VFunc::Type VFunc::type() const
{
    return service()->type(typeId());
}

uint8_t VFunc::typeId() const
{
    return m_typeParam->value().toUInt();
}

void VFunc::setType(uint8_t typeId)
{
    m_typeParam->updateValue(QString::number(typeId));
}

uint8_t VFunc::argValue() const
{
    return m_argParam->value().toUInt();
}

void VFunc::setArgValue(uint8_t value)
{
    m_argParam->updateValue(QString::number(value));
}

FuncService *VFunc::service() const
{
    return m_service;
}

DcParam *VFunc::typeParam() const
{
    return m_typeParam;
}

DcParam *VFunc::argParam() const
{
    return m_argParam;
}

void VFunc::setOutput(FuncServiceOutput *output)
{
    m_output = output;
}

FuncServiceOutput *VFunc::output() const
{
    return m_output;
}
