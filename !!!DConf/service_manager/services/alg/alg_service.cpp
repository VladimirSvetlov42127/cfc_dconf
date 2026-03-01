#include "alg_service.h"

AlgService::AlgService(uint16_t index, const QString &name, uint16_t addr)
    : Service{ name }
    , m_addr{addr}
    , m_index{index}
{
}

Service::Type AlgService::type() const
{
    return Service::AlgType;
}

uint16_t AlgService::addr() const
{
    return m_addr;
}

uint16_t AlgService::index() const
{
    return m_index;
}

AlgServiceInput *AlgService::makeInput(uint16_t id, DcParam *bindParam)
{
    auto sio = std::make_unique<AlgServiceInput>(id, inputs().size(), bindParam, this);
    auto raw = sio.get();
    Service::addInput(std::move(sio));

    // Запись в таблицу привязки для совместимости
    return raw;
}

AlgServiceOutput *AlgService::makeOutput(uint16_t id, DcParam *bindParam)
{
    auto sio = std::make_unique<AlgServiceOutput>(id, outputs().size(), bindParam, this);
    auto raw = sio.get();
    Service::addOutput(std::move(sio));

    // Запись в таблицу привязки для совместимости
    return raw;
}
