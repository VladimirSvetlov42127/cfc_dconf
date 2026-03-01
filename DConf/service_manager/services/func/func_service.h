#ifndef __FUNC_SERVICE_H__
#define __FUNC_SERVICE_H__

#include <unordered_map>

#include "service_manager/services/service.h"
#include "service_manager/services/func/vfunc.h"

// Сервис функций виртуальных выходов.
// Позволяет назначать на вирутальные выходы функции, и задавать значения их аргументов.
// Регулирует механизм привязки/отвязки вирутальных входов при использовании функций, аргументом которых является виртуальный вход.
// Для корректной работы сервису необходим список всех виртуальных входов.

class FuncService : public Service
{
public:
    // Структура с информацией о типе функции виртуального выхода
    struct VFuncInfo
    {
        uint8_t id;         // идентификатор типа для данного устройства.
        VFunc::Type type;   // тип функции из общего списка типов функций виртуальных выходов.
        QString name;       // имя функции
    };

    FuncService(const QString &name);
    virtual ~FuncService() = default;

    // Тип сервиса
    Service::Type type() const override;

    // Добавляет VDin в список регулируемых виртуальных входов.
    // используется при инциализации сервиса.
    void appendVDin(VirtualInputSignal *vdin);

    // Добавляет в список поддерживаемых функций виртуального выхода, тип функции type, под идентификатором типа id.
    // используется при инциализации сервиса.
    void appendSupportedType(uint8_t id, VFunc::Type type);

    // Создает функцию и устанавливает ее на виртуальный выход vdout.
    // используется при инциализации сервиса.
    // Если аргументом типа функции установленого в typeParam является vdin,
    // будет создан выход сервиса с попыткой привязки к vdin соответсвующего значению argParam
    VFunc* makeVirtualFunc(VirtualOutputSignal *vdout, DcParam *typeParam, DcParam *argParam);

    // Список поддерживаемых типов функций, в виде ассоциативного контейнера.
    // ключ - идентификатор типа, значение - информация о типе.
    const std::map<uint8_t, VFuncInfo>& supportedTypes() const;

    // Список всех функций виртуальных выходов
    const VFunc::UPtrVector& funcList() const;

    // Имя функции по идентификатору типа id
    QString name(uint8_t id) const;

    // Тип функции по идентификатору типа id
    VFunc::Type type(uint8_t id) const;

    // Установка типа функции вирутального выхода под индексом idx в тип с идентификатором typeId.
    void setType(uint8_t idx, uint8_t typeId);

    // Установка значения аргумента функции вирутального выхода под индексом idx, в значение value.
    void setArgValue(uint8_t idx, uint8_t value);

private:
    // Создание выходa сервиса для функции func.
    // Если будет найден vdin c subTypeId равным значению аргумента функции, и этот vdin будет свободен,
    // Выход сервиса будет привязан к этому vdin.
    void makeOutput(VFunc* func);

    // Удаление выходa сервиса для функции func.
    // Если есть vdin привязанный к этому выход сервиса, он будет освобождён.
    void removeOutput(VFunc* func);

    // Возвращает true, если аргуметном функции с идентификатором типа typeId, является виртуальный вход
    // в противном случае fasle.
    bool hasVDinArg(uint8_t typeId) const;

private:
    std::unordered_map<uint8_t, VirtualInputSignal*> m_vdins;
    std::map<uint8_t, VFuncInfo> m_vfuncsInfo;
    VFunc::UPtrVector m_vfuncList;
};

#endif // __FUNC_SERVICE_H__
