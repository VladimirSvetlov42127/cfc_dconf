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
    return QString("Вход: алг %1, пин %2").arg(m_parent->id()).arg(pin() + 1);
}

QString CfcServiceInput::text() const
{
    return source() ? source()->text() : name();
}

void CfcServiceInput::onSourceChanged(InputSignal *newSource, InputSignal *prevSource)
{
     // if (prevSource)
     //     qDebug() << QString("DELETE FROM matrix_alg_cfc WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(prevSource->globalID());

     // if (newSource)
     //      qDebug() << QString("INSERT INTO matrix_alg_cfc WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(newSource->globalID());
}
