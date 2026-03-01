#include "output_signal.h"

#include <QDebug>

#include "service_manager/signals/signal.h"
#include "service_manager/signals/input_signal.h"

OutputSignal::OutputSignal(int32_t id, int32_t internalid, uint8_t direction, uint8_t type, uint8_t subtype,const QString &name, DcParam* bindParam, uint16_t subtypeID)
    : Signal{id, internalid, direction, type, subtype, name, subtypeID}
    , TargetElement{bindParam}
{
}

QString OutputSignal::name() const
{
    return Signal::name();
}

void OutputSignal::onSourceChanged(InputSignal *newSource, InputSignal *prevSource)
{
     // if (prevSource)
     //     qDebug() << QString("DELETE FROM matrix_signals WHERE src_globalID=%1 AND dst_globalID=%2").arg(prevSource->globalID()).arg(globalID());

     // if (newSource)
     //     qDebug() << QString("INSERT INTO matrix_signals WHERE src_globalID=%1 AND dst_globalID=%2").arg(newSource->globalID()).arg(globalID());
}
