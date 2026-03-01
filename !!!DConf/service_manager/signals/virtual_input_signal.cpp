#include "virtual_input_signal.h"

#include "service_manager/services/service_output.h"
#include "service_manager/signals/target_element.h"

VirtualInputSignal::VirtualInputSignal(int32_t id, int32_t internalid, uint8_t direction, uint8_t type, uint8_t subtype, const QString &name, uint16_t subtypeID)
    : InputSignal{id, internalid, direction, type, subtype, name, subtypeID}
{
}

// QString VirtualInputSignal::name() const
// {
//     return source() ? source()->name() : Signal::name();
// }

QString VirtualInputSignal::text() const
{
    if (source())
        return QString("[%1] Виртуальный (%2)").arg(internalID()).arg(source()->name());

    QList<TargetElement*> targets_list = this->targets();
    if (!targets_list.isEmpty())
        return QString("[%1] -> | %2 |").arg(internalID()).arg(targets_list.at(0)->name());

    return QString("[%1] %2").arg(internalID()).arg(name());
}

QString VirtualInputSignal::fullText() const
{
    QList<TargetElement*> targets_list = this->targets();
    QString target_name = text();
    if (!source() && targets_list.isEmpty())
        return target_name;

    if (source() && targets_list.isEmpty())
        return target_name;

    if (source() && !targets_list.isEmpty()) {
        target_name = target_name + QString(" -> | ");
        for (int i = 0; i < targets_list.size(); i++)
            target_name = target_name + targets_list.at(i)->name() + " | ";
        return target_name;
    }

    for (int i = 1; i < targets_list.size(); i++)
        target_name = target_name + targets_list.at(i)->name() + " | ";

    return target_name;
}

void VirtualInputSignal::setSource(ServiceOutput *newSource)
{
    if (source() == newSource)
        return;

    auto prevSource = source();
    m_source = newSource;
    if (prevSource && prevSource->target() == this)
        prevSource->setTarget(nullptr);

    if (newSource)
        newSource->setTarget(this);
}

ServiceOutput *VirtualInputSignal::source() const
{
    return m_source;
}
