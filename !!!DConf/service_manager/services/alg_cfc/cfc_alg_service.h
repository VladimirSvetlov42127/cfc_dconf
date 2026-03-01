#ifndef CFCALGSERVICE_H
#define CFCALGSERVICE_H

#include "service_manager/services/service.h"
#include "service_manager/services/alg_cfc/cfc_service_input.h"
#include "service_manager/services/alg_cfc/cfc_service_output.h"

class CfcAlgService : public Service
{
public:
    // maxPinCount - максимальное количество ножек алгоритма для входов/выходов
    CfcAlgService(const QString &name, uint8_t id);
    virtual ~CfcAlgService() = default;

    Service::Type type() const override;

    // Добавление пина (ножки) к алгоритму, bindParam - параметр привязки. Если равен nullptr ничего не происходит
    void appendPin(DcParam* bindParam);

    // Возвращает параметр привязки соответсвующий pin, или nullptr если такого нет.
    DcParam* bindParam(uint8_t pin) const;

    // Ид алгоритма гибкой логики для табилц привязки. Для обратной совместимости
    uint8_t id() const;

    // Возвращает свободную ножку алгоритма, или -1 если нету свободных.
    int freePin() const;

    // Свободна ли ножка pin
    bool isFree(uint8_t pin) const;

    // Получение входа/выхода по пину. Если pin свободный или на нём противоположный элемент(вход/выход), вернёт nullptr
    CfcServiceInput* input(uint8_t pin) const;
    CfcServiceOutput* output(uint8_t pin) const;

    // Создает и добавляет вход/выход на ножку pin. Если pin уже занят добавления не происходит и возвращает nullptr.
    // Запись в таблицу привязки для совместимости
    CfcServiceInput* makeInput(uint16_t id, uint8_t pin);
    CfcServiceOutput* makeOutput(uint16_t id, uint8_t pin);

    // Удаление входа/выхода из сервиса. !!!ПОСЛЕ ВЫЗОВА ЭТОЙ ФУНКЦИИ, ВСЕ УКАЗАТЕЛИ НА ВХОД/ВЫХОД СТАНУТ НЕ ВАЛИДНЫМИ!!!
    // Запись в таблицу привязки для совместимости
    void removeInput(CfcServiceInput* input);
    void removeOutput(CfcServiceOutput* output);

private:
    uint8_t m_id;
    std::vector<std::pair<DcParam*, ServiceIO*>> m_ios;
};

#endif // CFCALGSERVICE_H
