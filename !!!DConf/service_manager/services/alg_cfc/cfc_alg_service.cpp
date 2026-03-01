#include "cfc_alg_service.h"

CfcAlgService::CfcAlgService(const QString &name, uint8_t id)
    : Service { name }
    , m_id{ id }
{
    m_ios.reserve(32);
}

Service::Type CfcAlgService::type() const
{
    return Service::CfcAlgType;
}

void CfcAlgService::appendPin(DcParam *bindParam)
{
    if (!bindParam)
        return;

    m_ios.emplace_back(bindParam, nullptr);
}

DcParam *CfcAlgService::bindParam(uint8_t pin) const
{
    if (pin >= m_ios.size())
        return nullptr;

    return m_ios.at(pin).first;
}

uint8_t CfcAlgService::id() const
{
    return m_id;
}

int CfcAlgService::freePin() const
{
    for(size_t i = 0; i < m_ios.size(); ++i)
        if (!m_ios.at(i).second)
            return i;

    return -1;
}

bool CfcAlgService::isFree(uint8_t pin) const
{
    if (pin >= m_ios.size())
        return false;

    return !m_ios.at(pin).second;
}

CfcServiceInput *CfcAlgService::input(uint8_t pin) const
{
    if (pin < m_ios.size()) {
        auto &[bindParam, sio] = m_ios.at(pin);
        if (sio)
            for(auto& in: inputs())
                if (in.get() == sio)
                    return static_cast<CfcServiceInput*>(sio);
    }

    return nullptr;
}

CfcServiceOutput *CfcAlgService::output(uint8_t pin) const
{
    if (pin < m_ios.size()) {
        auto &[bindParam, sio] = m_ios.at(pin);
        if (sio)
            for(auto& out: outputs())
                if (out.get() == sio)
                    return static_cast<CfcServiceOutput*>(sio);
    }

    return nullptr;
}

CfcServiceInput *CfcAlgService::makeInput(uint16_t id, uint8_t pin)
{
    if (!isFree(pin))
        return nullptr;

    auto &[bindParam, sio] = m_ios[pin];
    auto inputUPtr = std::make_unique<CfcServiceInput>(id, pin, bindParam, this);
    auto input = inputUPtr.get();
    sio = input;
    Service::addInput(std::move(inputUPtr));

    // Запись в таблицу привязки для совместимости
    return input;
}

CfcServiceOutput *CfcAlgService::makeOutput(uint16_t id, uint8_t pin)
{
    if (!isFree(pin))
        return nullptr;

    auto &[bindParam, sio] = m_ios[pin];
    auto outputUPtr = std::make_unique<CfcServiceOutput>(id, pin, bindParam, this);
    auto output = outputUPtr.get();
    sio = output;
    Service::addOutput(std::move(outputUPtr));

    // Запись в таблицу привязки для совместимости
    return output;
}

void CfcAlgService::removeInput(CfcServiceInput *input)
{
    auto &[bindParam, sio] = m_ios[input->pin()];
    sio = nullptr;

    Service::deleteInput(input);
    // Запись в таблицу привязки для совместимости
}

void CfcAlgService::removeOutput(CfcServiceOutput *output)
{
    auto &[bindParam, sio] = m_ios[output->pin()];
    sio = nullptr;

    Service::deleteOutput(output);
    // Запись в таблицу привязки для совместимости
}
