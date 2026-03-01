#include "dc_controller.h"

#include <unordered_map>

#include <dpc/sybus/channel/Channel.h>
//
//
#include <service_manager/service_manager.h>

using namespace Dpc::Sybus;

namespace {
	struct SpecialParam
	{
		bool base;
		bool updatable;
	};

	const std::unordered_map<uint16_t, SpecialParam> g_SpecialParams = {
        { SP_DEVICE_NAME,		{ true,  false } },	// Название устройства
        { SP_HDWPARTNUMBER,		{ true,  false } },	// Код устройства
        { SP_SOFTWARE_ID,		{ true,  false } },	// Код ПО
        { SP_SERNUM,			{ true,  false } },	// Серийный номер
        { SP_SOFTWARE_VERTION,	{ true,  true  } },	// Сборка ПО
        { SP_CFGVER,			{ true,  true  } },	// Версия конфигурации ПО устройства
        { SP_DOUT_FUNCLIST,		{ false, true  } },	// Список поддерживаемых функций виртуальных выходов
        { SP_61850_SUPLIST,     { false, true  } },	// Состав протокола 61850
        { SP_OSC_LIMITS,        { false, true  } },	// Особенности и ограничения осциллографа
	};

	const QHash<int, QString> g_VirtualFuncions = {
		{DcController::NOTUSE, "Не используется"},
		{DcController::TEST_CNTRL, "Пуск теста цепей управления"},
		{DcController::OSCILL_START, "Пуск осциллографирования"},
		{DcController::VDIN_CONTROL, "Управление виртуальным входом"},
		{DcController::XCBR_CNTRL, "Управление выключателем"},
		{DcController::XCBR_RZA_CNTRL, "Управление выключателем c РЗА"},
		{DcController::QUIT_CMD, "Квитация событий"},
		{DcController::FIX_VDIN, "Фиксация входа"},
		{DcController::VDOUT_CONFIRM, "Квитация события"},
		{DcController::VDIN_EVENT, "Событие в виртуальном входе"},
		{DcController::EXEC_EMBEDED_ALG, "Запуск встроенного алгоритма"},
		{DcController::NETWUSE,	"Штатное управление"},
		{DcController::CHANGE_SIM, "Смена СИМ карты"},
		{DcController::BLOCK_TU, "Запретить ТУ"}, 
		{DcController::CONTROL_SV, "Управление SV потоком"},
		{DcController::ACTIVE_GROUP, "Установка активной группы уставок"}
	};
}

DcController::DcController(int32_t index, QString filepath, QString name) : 
	m_index(index), m_path(filepath), m_name(name)
{
	m_algs_internal = new DcPoolSingleKey<DcAlgInternal*>();
	m_algs_cfc = new DcPoolSingleKey<DcAlgCfc*>();
	m_settings = new DcSettingsPool();
	m_params_cfg = new DcParamCfgPool<DcParamCfg*>();
	m_boards = new DcPoolSingleKey<DcBoard*>();
	m_matrix_alg = new DcMatrixPool<DcMatrixElementAlg*>();
	m_matrix_cfc = new DcMatrixPool<DcMatrixElementAlgCfc*>();
	m_matrix_signals = new DcMatrixPool<DcMatrixElementSignal*>();
    _service_manager = new ServiceManager(this);
}

DcController::~DcController()
{
	delete m_matrix_signals;
	delete m_matrix_cfc;
	delete m_matrix_alg;
	delete m_boards;
	delete m_params_cfg;
	delete m_settings;
	delete m_algs_cfc;
	delete m_algs_internal;
	delete _service_manager;
}

int32_t DcController::index() const
{
	return m_index;
}

QString DcController::path() const
{
	return m_path;
}

QString DcController::name() const
{
	return m_name;
}

DcPoolSingleKey<DcAlgCfc*>* DcController::algs_cfc() const
{
	return m_algs_cfc;
}

DcPoolSingleKey<DcAlgInternal*>* DcController::algs_internal() const
{
	return m_algs_internal;
}

DcParamCfgPool<DcParamCfg*> * DcController::params_cfg() const
{
	return m_params_cfg;
}

DcSettingsPool * DcController::settings() const
{
	return m_settings;
}

DcPoolSingleKey<DcBoard*>* DcController::boards() const
{
	return m_boards;
}

DcMatrixPool<DcMatrixElementAlg*>* DcController::matrix_alg() const
{
	return m_matrix_alg;
}

DcMatrixPool<DcMatrixElementAlgCfc*>* DcController::matrix_cfc() const
{
	return m_matrix_cfc;
}

DcMatrixPool<DcMatrixElementSignal*>* DcController::matrix_signals() const
{
	return m_matrix_signals;
}

ChannelPtr DcController::channel() const
{
	return m_channel;
}

void DcController::deleteChannel()
{
	m_channel->disconnect();
	m_channel.reset();
}

bool DcController::updateName(const QString &newname)
{
	if (newname == m_name) return true;

	m_name = newname;
	emit nameChanged(newname);
	return true;
}

bool DcController::setChannel(ChannelPtr channel)
{
	if (m_channel) m_channel->disconnect();
	m_channel = channel;
	return true;
}

DcParam * DcController::getParam(int32_t addr, int32_t idx, int32_t board) const
{
	if (board < 0) return params_cfg()->get(addr, idx);

	auto b = boards()->getById(board);
	if (!b)	return nullptr;
	return b->params()->get(addr, idx);
}

DcController::Type DcController::type() const
{
	auto model = settings()->get("model");
	QString value = model ? model->value().trimmed().toUpper() : QString();

	if (value.contains("DEPROTEC"))	return Deprotec;
	if (value == "DEPRTU-LT-GSM") return GSM;
	if (value.contains("DEPRTU-LT-")) return LT;
	if (value == "DEPRTU-P-SC")	return P_SC;
	if (value == "DEPRTU-P-SCI") return P_SCI;	
	if (value == "DEPRTU") return DepRtu;
	if (value == "EXRZA") return ExRza;
	if (value == "EXSW") return ExSW;
	if (value == "RTU3-M") return RTU3_M_P;
    if (value.startsWith("T2-")) return T2;
	if (value == "T2-DOUT-8R")	return T2_DOUT_8R;
	if (value == "T2-EM3M")		return T2_EM_3M;
	if (value == "RPR-485-T3") return RPR_485_T3;

	return Unknown;
}

bool DcController::hasEthernet() const
{
    auto ipParam = getParam(SP_IP4_ADDR, 0);
    if (!ipParam)
        return false;

    if (DcController::Deprotec == type()) {
        for(uint32_t i = 0; i < boards()->size(); ++i) {
            auto boardType = boards()->get(i)->type().trimmed();
            if ( "DRZA_2FX" == boardType || "DRZA_2TX" == boardType )
                return true;
        }

        return false;
    }

    return true;
}

uint DcController::rs485PortsCount() const
{
	auto param = dynamic_cast<DcParamCfg_v2*>(getParam(SP_USARTPRTPAR_BYTE, 0));
	if (!param)	return 0;

	return param->getProfileCount();
}

uint DcController::rs485SlavesMax() const
{
	auto param = dynamic_cast<DcParamCfg_v2*>(getParam(SP_USARTEXTNET_DEFINITION, 0));
	if (!param)	return 0;
	return param->getProfileCount();
}

QVariant DcController::getValue(int32_t addr, uint16_t index) const
{
	auto param = params_cfg()->get(addr, index);
	if (!param)
		return QVariant();

	return param->value();
}

bool DcController::setValue(int32_t addr, uint16_t index, const QVariant & value)
{
	auto param = params_cfg()->get(addr, index);
	if (!param)
		return false;

	param->updateValue(value.toString());
	return true;
}

QVariant DcController::getBitValue(int32_t addr, uint16_t bit, int profile, int32_t addrIndex) const
{	
	auto p = getParamForBit(addr, bit, profile, addrIndex);
	auto param = p.first;
	bit = p.second;

	if (!param || 0 > bit)
		return QVariant();

	return param->value().toUInt() & (1 << bit) ? Qt::Checked : Qt::Unchecked;
}

bool DcController::setBitValue(int32_t addr, uint16_t bit, const QVariant & value, int profile, int32_t addrIndex)
{
	auto p = getParamForBit(addr, bit, profile, addrIndex);
	auto param = p.first;
	bit = p.second;

	if (!param || 0 > bit)
		return false;	

	uint32_t val = param->value().toUInt();
	if (value.toBool())
		val |= 1 << bit;
	else
		val &= ~(1 << bit);

	param->updateValue(QString::number(val));
	return true;
}

QSet<uint16_t> DcController::specialParams(DcController::SpecialParamType type)
{
	QSet<uint16_t> result;
	for (auto &&it : g_SpecialParams) {
		switch (type) {
		case DcController::BaseParam: if (it.second.base) result.insert(it.first); break;
		case DcController::UpdatableParam: if (it.second.updatable) result.insert(it.first); break;
		default: result.insert(it.first); break;
		}
	}

	return result;
}

std::vector<std::pair<uint, QString>> DcController::virtualFunctionList() const
{
	std::vector<std::pair<uint, QString>> result;

	DcParamCfg_v2* vfListParam = dynamic_cast<DcParamCfg_v2*>(params_cfg()->get(SP_DOUT_FUNCLIST, 0));
	if (!vfListParam)
		return result;

	auto softIdParam = getParam(SP_SOFTWARE_ID, 0);
	if (!softIdParam)
		return result;

	auto softId = softIdParam->value().toUInt();
	auto isInvalidLtOscill = softId == DEPRTU_LT_T || softId == DEPRTU_LTA_T || softId == DEPRTU_LT_R || softId == DEPRTU_LTA_R || 
		softId == DEPRTU_LT_F || softId == DEPRTU_LTA_F;

	for (size_t i = 0; i < vfListParam->getSubProfileCount(); i++) {
		DcParamCfg* p = params_cfg()->get(SP_DOUT_FUNCLIST, i);
		auto funcId = p->value().toUInt();
		if (funcId == OSCILL_START && isInvalidLtOscill)
			continue;

		QString funcName = g_VirtualFuncions.value(funcId);
		if (funcName.isEmpty())
			continue;

		result.emplace_back(std::make_pair(i, funcName));
	}

	return result;
}

DcController::VirtualFunctionType DcController::virtualFunctionType(int functionIndex) const
{
	DcParamCfg_v2 *vfListParam = dynamic_cast<DcParamCfg_v2*>(params_cfg()->get(SP_DOUT_FUNCLIST, functionIndex));
	if (vfListParam) {
		int value = vfListParam->value().toUInt();
		if (g_VirtualFuncions.contains(value))
			return static_cast<VirtualFunctionType>(value);
	}

	return UNKNOWN;
}

bool DcController::isConnectionDiscret(DcSignal * signal) const
{
	if (signal->type() != DEF_SIG_TYPE_DISCRETE)
		return false;

	auto connectionDiscretParam = dynamic_cast<DcParamCfg_v2*>(getParam(SP_INDCON_OUTWORD_PARAM, 0));
	if (connectionDiscretParam)
		for (size_t i = 0; i < connectionDiscretParam->getSubProfileCount(); i++) {
			auto p = getParam(SP_INDCON_OUTWORD_PARAM, i);
			if (p->value().toUInt() == signal->internalId())
				return true;
		}

	return false;
}

bool DcController::isVirtualFunctionParamValue(VirtualFunctionType funcType, int value) const
{
	auto virtualFunctionsParam = dynamic_cast<DcParamCfg_v2*>(getParam(SP_DOUT_TYPE, 0));
	if (virtualFunctionsParam)
		for (size_t i = 0; i < virtualFunctionsParam->getProfileCount(); i++) {
			auto p = getParam(SP_DOUT_TYPE, i * PROFILE_SIZE);
			if (funcType != virtualFunctionType(p->value().toUInt()))
				continue;

			p = getParam(SP_DOUT_TYPE, i * PROFILE_SIZE + 1);
			if (p->value().toUInt() == value)
				return true;
		}

	return false;
}

int DcController::virtualDiscreteIndex(DcSignal * virtualSignal) const
{
	int index = 0;
	for (auto s: getSignalList(DEF_SIG_TYPE_DISCRETE, DEF_SIG_SUBTYPE_VIRTUAL)) {
		if (s == virtualSignal)
			return index;

		index++;		
	}

	return -1;
}

QVariant DcController::getTimeOffset() const
{
	auto p = getParam(SP_TIMEZONE_HOUR, 0);
	if (p)
		return p->value().toInt();

	p = getParam(SP_PROTOCOL, 0);
	if (!p)
		return QVariant();

	auto syncProtocol = p->value().toUShort();
	if (syncProtocol < Channel::SNTP) {
		if (p = getParam(SP_TIMESYNCROPARS, 13); p)
			return p->value().toInt();
	}
	else if (syncProtocol == Channel::SNTP) {
		if (p = getParam(SP_TIMESYNCROPARS, 256 + 6); p)
			return p->value().toInt();
	}

	return QVariant();
}

bool DcController::addSignal(DcSignal* signal, bool fromDb)
{
	auto container = getContainer(signal->type(), signal->direction());
	if (!container)
		return false;

	auto insertIt = container->insert({ signal->internalId(), signal });
	if (insertIt.second && !fromDb)
		signal->insert();
	return insertIt.second;
}

bool DcController::removeSignal(int32_t internalId, DefSignalType type, DefSignalDirection direction)
{
	auto container = getContainer(type, direction);
	if (!container)
		return false;

	auto findIt = container->find(internalId);
	if (findIt == container->end())
		return false;

	findIt->second->remove();
	container->erase(findIt);
	return true;
}

DcSignal* DcController::getSignal(int32_t index) const
{
	auto searchContainer = [](int32_t index, const SignalContainer &container) -> DcSignal* {
		for (auto &it: container)
			if (it.second->index() == index)
				return it.second;
		return nullptr;
	};

	if (auto res = searchContainer(index, m_inDiscrets); res)
		return res;
	if (auto res = searchContainer(index, m_outDiscrets); res)
		return res;
	if (auto res = searchContainer(index, m_inAnalogs); res)
		return res;
	if (auto res = searchContainer(index, m_inCounters); res)
		return res;

	return nullptr;
}

DcSignal* DcController::getSignal(int32_t internalId, DefSignalType type, DefSignalDirection direction) const
{
	auto container = getContainer(type, direction);
	if (!container)
		return nullptr;

	auto findIt = container->find(internalId);
	if (findIt == container->end())
		return nullptr;

	return findIt->second;
}

QList<DcSignal*> DcController::getSignalList() const
{
	QList<DcSignal*> result;
	auto collectSignals = [&result](const SignalContainer& container) {
		for (auto& s : container)
			result << s.second;
	};

	collectSignals(m_inDiscrets);
	collectSignals(m_inAnalogs);
	collectSignals(m_inCounters);
	collectSignals(m_outDiscrets);
	return result;
}

QList<DcSignal*> DcController::getSignalList(DefSignalType type, DefSignalSubType subType, DefSignalDirection direction) const
{
	QList<DcSignal*> result;
	auto container = getContainer(type, direction);
	if (container)
		for (auto& it : *container) {
			auto s = it.second;
			if (DEF_SIG_SUBTYPE_UNDEF == subType || s->subtype() == subType)
				result << it.second;
		}

	return result;
}

DcController::SignalContainer* DcController::getContainer(DefSignalType type, DefSignalDirection direction) const
{
	switch (type)
	{
	case DEF_SIG_TYPE_DISCRETE:
		if (direction == DEF_SIG_DIRECTION_INPUT)
			return const_cast<SignalContainer*>(&m_inDiscrets);
		else if (direction == DEF_SIG_DIRECTION_OUTPUT)
			return const_cast<SignalContainer*>(&m_outDiscrets);
		break;
	case DEF_SIG_TYPE_ANALOG:
		return const_cast<SignalContainer*>(&m_inAnalogs);
	case DEF_SIG_TYPE_COUNTER:
		return const_cast<SignalContainer*>(&m_inCounters);
	default:
		break;
	}

	return nullptr;
}

std::pair<DcParam*, int> DcController::getParamForBit(int addr, int bit, int profile, int addrIndex) const
{
	int indexBase = profile * PROFILE_SIZE;
	if (addrIndex > -1) {
		auto param = getParam(addr, addrIndex + indexBase);
		return { param, bit };
	}

	auto param = getParam(addr, 0);
	if (!param)
		return { nullptr, -1 };

	uint16_t bitsPerParam = 0; // количество битов в параметре
	switch (param->type()) {
	case DEF_DATA_TYPE_BYTE:
	case DEF_DATA_TYPE_CHAR:
	case DEF_DATA_TYPE_BYTEBOOL:
	case DEF_DATA_TYPE_BOOL:
	case DEF_DATA_TYPE_T_8BIT:
	case DEF_DATA_TYPE_T_SBYTE:
		bitsPerParam = 8;
		break;
	case DEF_DATA_TYPE_SHORT:
	case DEF_DATA_TYPE_T_16BIT:
	case DEF_DATA_TYPE_WORD:
		bitsPerParam = 16;
		break;
	case DEF_DATA_TYPE_INTEGER:
	case DEF_DATA_TYPE_DWORD:
	case DEF_DATA_TYPE_LONG:
	case DEF_DATA_TYPE_T_32BIT:
		bitsPerParam = 32;
		break;
	default: return { nullptr, -1 };
	}

	addrIndex = (bit / bitsPerParam) + indexBase; // индекс параметра в котрорм содержится нужный бит.
	bit = bit % bitsPerParam; // номер бита в параметре.
	param = getParam(addr, addrIndex);
	return { param, bit };
}

void DcController::loadServiceManager()
{
    int service_version = 0;
    if (!settings()->exist("service_version")) {
        settings()->add(new DcSetting (this->index(), "service_version", "0"), false);
    }

    service_version = settings()->get("service_version")->value().toInt();
    _service_manager->load(service_version);

    if (service_version <= 0) service_version = 1;
    settings()->get("service_version")->updateValue(QString::number(service_version));
}
