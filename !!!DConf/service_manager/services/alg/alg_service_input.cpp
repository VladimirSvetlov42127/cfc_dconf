#include "alg_service_input.h"

#include <QDebug>

#include "service_manager/signals/input_signal.h"

AlgServiceInput::AlgServiceInput(uint16_t id, uint8_t pin, DcParam *bindParam, Service *service)
    : ServiceInput{id, pin, bindParam, service}
{
}

void AlgServiceInput::onSourceChanged(InputSignal *newSource, InputSignal *prevSource)
{
     // if (prevSource)
     //     qDebug() << QString("DELETE FROM matrix_alg WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(prevSource->globalID());

     // if (newSource)
     //      qDebug() << QString("INSERT INTO matrix_alg WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(newSource->globalID());
}
