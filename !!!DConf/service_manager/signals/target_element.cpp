#include "target_element.h"

#include "data_model/storage_for_controller_params/dc_param.h"

namespace {

const uint16_t g_NoSignal = 0xFFFF;

} // namespace

TargetElement::TargetElement(DcParam *bindParam, Service* service)
    : m_service{service}
    , m_bindParam{bindParam}
{
}

const Service *TargetElement::service() const
{
    return m_service;
}

void TargetElement::setSource(InputSignal *newSource)
{
    if (source() == newSource)
        return;

    auto prevSource = source();
    m_source = newSource;
    if (prevSource)
        prevSource->removeTarget(this);    

    if (newSource)
        newSource->addTarget(this);

    auto singalInternalId = newSource ? newSource->internalID() : g_NoSignal;
    m_bindParam->updateValue(QString::number(singalInternalId));

    onSourceChanged(newSource, prevSource);
}

InputSignal *TargetElement::source() const
{
    return m_source;
}

DcParam *TargetElement::bindParam() const
{
    return m_bindParam;
}

void TargetElement::onSourceChanged(InputSignal *, InputSignal *)
{
}

