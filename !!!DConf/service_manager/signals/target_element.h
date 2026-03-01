#ifndef __TARGET_ELEMENT_H__
#define __TARGET_ELEMENT_H__

#include <QString>

#include "service_manager/signals/input_signal.h"

// TargetElement - Базовый элемент привязки типа "Цель" - элемент привязки которой может участвовать в привязках, только как цель - Вход сервиса!!! или Дискретный выход!!!
// У него может быть задан только один "Источник (сигнал)" - Дискретный вход.
// При установлении/сбросе привязок выставляет параметру bindParam - корректное значение

class DcParam;
class Service;

class TargetElement
{
public:
    // bindParam - физический параметр с значением привязки
    // Если TargetElement - это вход сервиса, то service должен указывать на сервис, в противном случае nullptr.
    TargetElement(DcParam* bindParam, Service* service = nullptr);
    virtual ~TargetElement() = default;

    virtual QString name() const = 0;

    // Если TargetElement - это вход сервиса, то возвращает сервис которому принадлежит этот вход, в противном случае nullptr.
    const Service* service() const;

    void setSource(InputSignal* newSource);
    InputSignal* source() const;

protected:
    DcParam* bindParam() const;
    virtual void onSourceChanged(InputSignal *newSource, InputSignal *prevSource);

private:
    Service *m_service = nullptr;
    DcParam* m_bindParam;
    InputSignal* m_source = nullptr;
};

#endif // __TARGET_ELEMENT_H__
