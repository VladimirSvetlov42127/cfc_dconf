#include "virtual_output_signal.h"

#include "service_manager/services/func/vfunc.h"

VirtualOutputSignal::VirtualOutputSignal(int32_t id, int32_t internalid, uint8_t direction, uint8_t type, uint8_t subtype, const QString &name, DcParam *bindParam, uint16_t subtypeID)
    : OutputSignal{id, internalid, direction, type, subtype, name, bindParam, subtypeID}
    , m_func{ nullptr }
{

}

VFunc *VirtualOutputSignal::func() const
{
    return m_func;
}

void VirtualOutputSignal::setFunc(VFunc *func)
{
    m_func = func;
}

QString VirtualOutputSignal::text() const
{
    if (!func())
        return OutputSignal::text();

    if (func()->type() == VFunc::NOTUSE)
        return QString("Не используется");

    return QString("[%1] %2").arg(internalID()).arg(func()->text());
}
