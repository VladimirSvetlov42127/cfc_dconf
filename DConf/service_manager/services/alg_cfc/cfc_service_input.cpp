#include "cfc_service_input.h"

#include <QDebug>

#include "service_manager/signals/input_signal.h"
#include "service_manager/services/alg_cfc/cfc_alg_service.h"

CfcServiceInput::CfcServiceInput(uint16_t id, uint8_t pin, DcParam *bindParam, CfcAlgService *service)
    : ServiceInput{id, pin, bindParam, service},
    m_parent { service }
{
}

QString CfcServiceInput::name() const
{
    //return source() ? source()->name() : QString("Pin %1: вход").arg(pin());
    return QString("Алг %1, пин %2 (Вход)").arg(m_parent->id() + 1).arg(pin() + 1);
}

void CfcServiceInput::onSourceChanged(InputSignal *newSource, InputSignal *prevSource)
{
     // if (prevSource)
     //     qDebug() << QString("DELETE FROM matrix_alg_cfc WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(prevSource->globalID());

     // if (newSource)
     //      qDebug() << QString("INSERT INTO matrix_alg_cfc WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(newSource->globalID());
}
