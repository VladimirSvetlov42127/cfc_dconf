#include "cfc_service_output.h"

#include <QDebug>

#include "service_manager/signals/virtual_input_signal.h"
#include "service_manager/services/alg_cfc/cfc_alg_service.h"

CfcServiceOutput::CfcServiceOutput(uint16_t id, uint8_t pin, DcParam *bindParam, CfcAlgService *service)
    : ServiceOutput { id, pin, bindParam, service }
    , m_parent{service}
{
}

QString CfcServiceOutput::name() const
{
    return QString("Выход: алг %1, пин %2").arg(m_parent->id()).arg(pin() + 1);
}

QString CfcServiceOutput::text() const
{
    return target() ? target()->text() : name();
}

void CfcServiceOutput::onTargetChanged(VirtualInputSignal *newTarget, VirtualInputSignal *prevTarget)
{
     // if (prevTarget)
     //     qDebug() << QString("DELETE FROM matrix_alg_cfc WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(prevTarget->globalID());

     // if (newTarget)
     //      qDebug() << QString("INSERT INTO matrix_alg_cfc WHERE algIoID=%1 AND globalID=%2").arg(id()).arg(newTarget->globalID());
}
