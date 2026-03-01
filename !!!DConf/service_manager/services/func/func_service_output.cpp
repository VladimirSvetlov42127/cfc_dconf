#include "func_service_output.h"

#include "data_model/storage_for_controller_params/dc_param.h"

namespace {
    const uint16_t g_NoSignal = 0xFF;
} // namespace

FuncServiceOutput::FuncServiceOutput(const QString &name, DcParam* bind_param, Service *service)
    : ServiceOutput{ 0, 0, bind_param, service },
    _name{ name }
{
}

void FuncServiceOutput::updateBinding(VirtualInputSignal *new_target)
{
    auto newValue = new_target ? new_target->subTypeID() : g_NoSignal;
    bindParam()->updateValue(QString::number(newValue));
}
