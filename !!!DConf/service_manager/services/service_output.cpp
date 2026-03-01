#include "service_output.h"

#include "data_model/storage_for_controller_params/dc_param.h"

namespace {
    const uint16_t g_NoSignal = 0xFFFF;
} // namespace

ServiceOutput::ServiceOutput(uint16_t id, uint8_t pin, DcParam *bindParam, Service *service)
    : ServiceIO{id, pin}
    , m_service{service}
    , m_bindParam{bindParam}
{
}

const Service *ServiceOutput::service() const
{
    return m_service;
}

QString ServiceOutput::name() const
{
    return m_bindParam->name();
}

void ServiceOutput::setTarget(VirtualInputSignal *newTarget)
{
    if (target() == newTarget)
        return;

    auto prevTarget = target();
    m_target = newTarget;
    if (prevTarget && prevTarget->source() == this)
        prevTarget->setSource(nullptr);

    if (newTarget)
        newTarget->setSource(this);

    updateBinding(newTarget);
    onTargetChanged(newTarget, prevTarget);
}

void ServiceOutput::onTargetChanged(VirtualInputSignal*, VirtualInputSignal*)
{
}

VirtualInputSignal *ServiceOutput::target() const
{
    return m_target;
}

DcParam *ServiceOutput::bindParam() const
{
    return m_bindParam;
}

void ServiceOutput::updateBinding(VirtualInputSignal *new_target)
{
    auto newValue = new_target ? new_target->internalID() : g_NoSignal;
    bindParam()->updateValue(QString::number(newValue));
}

