#ifndef __SERVICE_MANAGER_H__
#define __SERVICE_MANAGER_H__

#include <map>
#include <QObject>

#include "service_manager/services/service.h"
#include "service_manager/services/alg/alg_service.h"
#include "service_manager/services/alg_cfc/cfc_alg_service.h"
#include "service_manager/signals/input_signal.h"
#include "service_manager/signals/virtual_input_signal.h"
#include "service_manager/signals/output_signal.h"

class DcController;

//===================================================================================================================================================
//	Описание класса
//===================================================================================================================================================
class ServiceManager : public QObject
{
    Q_OBJECT

public:
    //===============================================================================================================================================
    //	Конструктор класса
    //===============================================================================================================================================
    ServiceManager(DcController *controller);

    //===============================================================================================================================================
    //	Открытые методы класса
    //===============================================================================================================================================
    bool load(uint8_t mode);
    bool configure();

    QList<InputSignal*> dins() const;
    QList<OutputSignal*> douts() const;
    QList<VirtualInputSignal*> vdins() const;
    QList<VirtualInputSignal*> freeVdins() const;
    QList<VirtualInputSignal*> busyVdins() const;
    InputSignal* din(uint16_t internalID) const;

    QList<Service*> services() const;
    QList<AlgService*> algServices() const;
    QList<CfcAlgService*> cfcAlgServices() const;
    AlgService* alg(uint16_t addr) const;

    CfcAlgService* CfcAlg(uint8_t id) const;
    CfcAlgService* appendCfcService(const QString &name, uint8_t id, uint8_t profile);
    void removeCfcService(uint8_t id);
    uint16_t nextCfcID() { return ++m_cfcIoId; }

    //===============================================================================================================================================
    //	Отладочный вывод
    //===============================================================================================================================================
    void printAlgs();
    void printDouts();
    void printDins();

signals:
    //===============================================================================================================================================
    //	Сигналы для логирования
    //===============================================================================================================================================
    void infoToLog(const QString& message);
    void warningToLog(const QString& message);
    void errorToLog(const QString& message);

private:
    //===============================================================================================================================================
    //	Вспомогательные методы класса
    //===============================================================================================================================================
    DcController* controller() { return m_controller; }
    void setBindingParams();
    void clear();
    bool init();
    bool loadSignals();
    bool loadAlgs();
    bool loadCfcAlgs();
    bool loadFuncs();
    bool loadAlg(uint16_t address, const QString &name);

private:
    //===============================================================================================================================================
    //	Свойства класса
    //===============================================================================================================================================
    DcController *m_controller;
    std::map<uint16_t, std::unique_ptr<InputSignal>> m_dins;
    std::map<uint16_t, std::unique_ptr<OutputSignal>> m_douts;
    std::vector<std::unique_ptr<Service>> m_services;

    uint16_t m_algId;
    uint16_t m_algIoId;
    uint16_t m_cfcIoId;
};

#endif // __SERVICE_MANAGER_H__
