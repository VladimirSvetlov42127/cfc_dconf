#include "alg_service_output.h"

#include <QDebug>

#include "service_manager/signals/virtual_input_signal.h"

AlgServiceOutput::AlgServiceOutput(uint16_t id, uint8_t pin, DcParam *bindParam, Service *service)
    : ServiceOutput{id, pin, bindParam, service}
{
}

void AlgServiceOutput::onTargetChanged(VirtualInputSignal *newTarget, VirtualInputSignal *prevTarget)
{
     // if (prevTarget)
     //     qDebug() << QString("DELETE FROM matrix_alg WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(prevTarget->globalID());

     // if (newTarget)
     //      qDebug() << QString("INSERT INTO matrix_alg WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(newTarget->globalID());
}
