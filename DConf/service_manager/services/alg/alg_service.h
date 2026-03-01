#ifndef ALGSERVICE_H
#define ALGSERVICE_H

#include "service_manager/services/service.h"
#include "service_manager/services/alg/alg_service_input.h"
#include "service_manager/services/alg/alg_service_output.h"

class AlgService : public Service
{
public:
    AlgService(uint16_t index, const QString &name, uint16_t addr);
    virtual ~AlgService() = default;

    Service::Type type() const override;

    // Адрес встроенного алгоритма. Для идентификации сервиса.
    uint16_t addr() const;

    // Индекс алгоритма для табилц привязки. Для обратной совместимости
    uint16_t index() const;

    // Создает и добавляет вход/выход алгоритма
    // Запись в таблицу привязки для совместимости
    AlgServiceInput* makeInput(uint16_t id, DcParam *bindParam);
    AlgServiceOutput* makeOutput(uint16_t id, DcParam *bindParam);

private:
    uint16_t m_addr;
    uint16_t m_index;
};

#endif // ALGSERVICE_H
