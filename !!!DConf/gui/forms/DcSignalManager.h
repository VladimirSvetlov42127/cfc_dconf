#pragma once

#include <qobject.h>
#include <qdebug.h>
#include <qlist.h>
#include <qpair.h>

#include <data_model/dc_signal.h>
#include <data_model/dc_controller.h>

typedef QList< QPair<QString, QList<DcSignal*> > > formedListBySignalType_t;

// Класс используется как интерфейс между графическим редактором гибкой логики и конфигуратором
class DcSignalManager : public QObject
{
	Q_OBJECT

public:
	DcSignalManager(DcController *device, DcAlgCfc *alg = nullptr, QObject *parent = nullptr);

	DcController* device() const;
	DcAlgCfc* alg() const;

	formedListBySignalType_t getSignalList(DefSignalDirection dir);
	bool signalFree(DcSignal *signal);
	bool signalBusy(DcSignal *signal, int32_t &ioid, int32_t &algpos, QString &virtName);
	bool signalReplaced(int32_t signalIOID, DcSignal *signalNew, QString name);
    DcSignal* getSignal(int32_t ioid) const;

    int32_t getNewCfcIo();
    int32_t getFreeCfcAlgPin(int32_t algindex);

    DcSignal *getFreeVDin();
private:
	//===============================================================================================================================================
	//	Вспомогательные методы класса
	//===============================================================================================================================================
	bool isAlreadyBinded(DcSignal * psignal);
	bool CheckAinsCompare(int signal_idx);
//	bool CheckDiscrete(int signal_idx);

	//===============================================================================================================================================
	//	Свойства класса
	//===============================================================================================================================================
	DcController *m_device;
	DcAlgCfc *m_alg;
};
