#pragma once

#include <stdint.h>
#include <qvariant.h>
#include <qset.h>
#include <memory>

#include <data_model/dc_pool.h>
#include <data_model/dc_alg_internal.h>
#include <data_model/dc_alg_cfc.h>
#include <data_model/dc_signal.h>
#include <data_model/dc_settings_pool.h>
#include <data_model/dc_board.h>
#include <data_model/dc_matrix_pool.h>
#include <data_model/dc_matrix_element_alg.h>
#include <data_model/dc_matrix_element_alg_cfc.h>
#include <data_model/dc_matrix_element_signal.h>
#include <data_model/storage_for_controller_params/dc_param_cfg_pool.h>
#include <data_model/storage_for_controller_params/DcParamCfg_v2.h>

#include <dpc/sybus/smparlist.h>

#include "service_manager/service_manager.h"

namespace Dpc::Sybus
{
	class Channel;
}

using ChannelPtr = std::shared_ptr<Dpc::Sybus::Channel>;

//	Добавление менеджера сервисов
class ServiceManager;

//	Класс DcController
class DcController : public QObject
{
	Q_OBJECT
public:
	enum Type {
		Unknown,
		Deprotec, 
		LT,
		GSM,
		P_SC, 
		P_SCI,
		DepRtu,
		ExRza, 
		ExSW,
		RTU3_M_P,
        T2,
		T2_DOUT_8R,
		T2_EM_3M,
		RPR_485_T3,

	};

	enum Rs485Protocol {
		NotSet = 0x0,
		SybusSlaveProtocol,
		Iec101SlaveProtocol,
		ModbusSlaveProtocol,
		SybusMasterProtocol,
		Iec101MasterProtocol,
		ModbusMasterPrototcol,
		HayesModemMasterProtocol,
		Iec103SlaveProtocol,
		SpodesSlaveProtocol,
		SpodesMasterProtocol,
		ModemNanotronProtocol,
		ComPortProtocol
	};

	enum SpecialParamType {
		AllParam,
		BaseParam, 
		UpdatableParam
	};

	enum VirtualFunctionType {
		NOTUSE = 0,				// "Не используется";
		TEST_CNTRL,				// "Пуск теста цепей управления";				
		OSCILL_START,			// "Пуск осциллографирования";				
		VDIN_CONTROL,			// "Управление виртуальным входом";	
		XCBR_CNTRL,				// "Управление выключателем";				
		XCBR_RZA_CNTRL,			// "Управление выключателем c РЗА";			
		QUIT_CMD,				// "Квитация событий";							
		FIX_VDIN,				// "Фиксация входа";	
		VDOUT_CONFIRM,			// "Квитация события";
		VDIN_EVENT,				// "Событие в виртуальном входе";
		EXEC_EMBEDED_ALG,		// "Запуск встроенного алгоритма";	
		NETWUSE,				// "Штатное управление";
		CHANGE_SIM,				// "Смена СИМ карты";					
		BLOCK_TU,				// "Запретить ТУ";
		CONTROL_SV = 15,		// "Управление SV потоком"
		ACTIVE_GROUP,			// "Активная группа уставок"

		UNKNOWN
	};

	DcController(int32_t index, QString filepath, QString name);
	virtual ~DcController();

	int32_t index() const;
	QString path() const;
	QString name() const;

	DcPoolSingleKey<DcAlgCfc*>* algs_cfc() const;
	DcPoolSingleKey<DcAlgInternal*>* algs_internal() const;
	virtual DcParamCfgPool<DcParamCfg*> *params_cfg() const;
	DcSettingsPool *settings() const;
	DcPoolSingleKey<DcBoard*> *boards() const;
	DcMatrixPool<DcMatrixElementAlg*> *matrix_alg() const;
	DcMatrixPool<DcMatrixElementAlgCfc*> *matrix_cfc() const;
	DcMatrixPool<DcMatrixElementSignal*> *matrix_signals() const;

	ChannelPtr channel() const;
	void deleteChannel();

	bool updateName(const QString &newname);
	void insert() {}
	void remove() {}
	bool setChannel(ChannelPtr channel);

	DcParam* getParam(int32_t addr, int32_t idx, int32_t board = -1) const;
    Type type() const;
    bool hasEthernet() const;

	uint rs485PortsCount() const;
	uint rs485SlavesMax() const;

	QVariant getValue(int32_t addr, uint16_t index) const;
	bool setValue(int32_t addr, uint16_t index, const QVariant &value);
	QVariant getBitValue(int32_t addr, uint16_t bit, int profile = 0, int32_t addrIndex = -1) const;
	bool setBitValue(int32_t addr, uint16_t bit, const QVariant &value, int profile = 0, int32_t addrIndex = -1);

	static QSet<uint16_t> specialParams(DcController::SpecialParamType type = AllParam);

	std::vector<std::pair<uint, QString>> virtualFunctionList() const;
	VirtualFunctionType virtualFunctionType(int functionIndex) const;
	bool isConnectionDiscret(DcSignal * signal) const;
	bool isVirtualFunctionParamValue(VirtualFunctionType funcType, int value) const;
	int virtualDiscreteIndex(DcSignal *virtualSignal) const;

	QVariant getTimeOffset() const;

	// Сигналы
	bool addSignal(DcSignal* signal, bool fromDb);
	bool removeSignal(int32_t internalId, DefSignalType type, DefSignalDirection direction = DEF_SIG_DIRECTION_INPUT);	
	DcSignal* getSignal(int32_t index) const;
	DcSignal* getSignal(int32_t internalId, DefSignalType type, DefSignalDirection direction = DEF_SIG_DIRECTION_INPUT) const;
	QList<DcSignal*> getSignalList() const;
	QList<DcSignal*> getSignalList(DefSignalType type, DefSignalSubType subType = DEF_SIG_SUBTYPE_UNDEF, DefSignalDirection direction = DEF_SIG_DIRECTION_INPUT) const;

signals:
	void nameChanged(const QString &name);

private:
	using SignalContainer = std::map<int32_t, DcSignal*>;
	SignalContainer* getContainer(DefSignalType type, DefSignalDirection direction = DEF_SIG_DIRECTION_INPUT) const;

	// Возвращает пару, искомый параметр и номер бита в нём
	std::pair<DcParam*, int> getParamForBit(int addr, int bit, int profile, int addrIndex) const;

private:
	int32_t m_index;
	QString m_path;
	QString m_name;
	DcSettingsPool *m_settings;
	DcPoolSingleKey<DcAlgInternal*> *m_algs_internal;
	DcPoolSingleKey<DcAlgCfc*> *m_algs_cfc;
	DcParamCfgPool<DcParamCfg*> *m_params_cfg;
	DcPoolSingleKey<DcBoard*> *m_boards;
	DcMatrixPool<DcMatrixElementAlg*> *m_matrix_alg;
	DcMatrixPool<DcMatrixElementAlgCfc*> *m_matrix_cfc;
	DcMatrixPool<DcMatrixElementSignal*> *m_matrix_signals;
	ChannelPtr m_channel;
	SignalContainer m_inDiscrets;
	SignalContainer m_inAnalogs;
	SignalContainer m_inCounters;
	SignalContainer m_outDiscrets;

public:
	//===============================================================================================================================================
	//	Дополнительные методы класса
	//===============================================================================================================================================
	const SignalContainer& inDiscrets() { return m_inDiscrets; }
	const SignalContainer& outDiscrets() { return m_outDiscrets; }
	void loadServiceManager();
    ServiceManager* serviceManager() const { return _service_manager; }

private:
	//===============================================================================================================================================
	//	Свойства класса
	//===============================================================================================================================================
    ServiceManager* _service_manager = nullptr;

};

