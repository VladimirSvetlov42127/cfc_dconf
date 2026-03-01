#include "service_input.h"

#include "data_model/storage_for_controller_params/dc_param.h"

ServiceInput::ServiceInput(uint16_t id, uint8_t pin, DcParam *bindParam, Service *service)
    : ServiceIO{id, pin}
    , TargetElement{bindParam, service}
{
}

QString ServiceInput::name() const
{
    return bindParam()->name();
}
