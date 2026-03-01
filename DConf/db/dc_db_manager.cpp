#include "dc_db_manager.h"

#include <qdebug.h>

#include <journal/JSource.h>

#include "data_model/storage_for_controller_params/DcController_v2.h"

// #include "data_model/dc_data_manager.h"

namespace {

const char* SELECT_SETTINGS             = "SELECT * FROM settings";
const char* SELECT_CFG_HEADLINE         = "SELECT * FROM cfg_parameters_headline";
const char* SELECT_CFG_ITEMS            = "SELECT * FROM cfg_parameters_items";
const char* SELECT_BOARDS               = "SELECT * FROM boards";
const char* SELECT_BOARD_CFG_PARAMETERS = "SELECT * FROM board_cfg_parameters";
const char* SELECT_SIGNALS              = "SELECT * FROM signals";
const char* SELECT_ALGS                 = "SELECT * FROM algs ORDER BY name";
const char* SELECT_ALG_IO               = "SELECT * FROM alg_io";
const char* SELECT_ALGS_CFC             = "SELECT * FROM algs_cfc";
const char* SELECT_ALG_CFC_IO           = "SELECT * FROM alg_cfc_io";
const char* SELECT_MATRIX_ALG           = "SELECT * FROM matrix_alg";
const char* SELECT_MATRIX_ALG_CFC       = "SELECT * FROM matrix_alg_cfc";
const char* SELECT_MATRIX_SIGNALS       = "SELECT * FROM matrix_signals";

int callback_settings_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 2)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr))
        return 0;

    QString setting_name = argv[0];
    QString setting_value = argv[1];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcSetting *psetting = new DcSetting(pcontroller->index(), setting_name, setting_value);
    if (!pcontroller->settings()->add(psetting, true)) {
        delete psetting;
        psetting = nullptr;
        return 0;
    }
    return 0;
}

int callback_cfg_parameters_headline(void *data, int colomns_count, char **colonm_data_pointer, char **colomn_name_pointer) {

    if (data == nullptr)
        return 0;

    if (colomns_count < 7) {
        //TODO to log transaction error
        return 0;
    }

    if ((colonm_data_pointer[0] == nullptr) || (colonm_data_pointer[1] == nullptr)) {
        //TODO to log error
        return 0;
    }

    int colomnInd = 0;	// Переменная для инкрементирования позиции в буфере
    QString name = colonm_data_pointer[colomnInd++];
    int dtype = atoi(colonm_data_pointer[colomnInd++]);
    int addr = atoi(colonm_data_pointer[colomnInd++]);

    uint16_t attributes = 0;
    QString flagstr;
    if (colonm_data_pointer[colomnInd] != nullptr)
        flagstr = colonm_data_pointer[colomnInd++];

    attributes = getAttributeIntFromString(flagstr);	// Преобразуем строку атрибутов в число


    uint32_t profiles_count = atoi(colonm_data_pointer[colomnInd++]);
    uint32_t subProfiles_count = atoi(colonm_data_pointer[colomnInd++]);
    uint16_t size_in_bytes = atoi(colonm_data_pointer[colomnInd++]);

    auto pcontroller = static_cast<DcController_v2*>(data);
    QSharedPointer<DcParamCfg_v2_Headline> param(new (DcParamCfg_v2_Headline)(pcontroller->index(), name, (DefParamDataType)dtype, addr, attributes, profiles_count, subProfiles_count, size_in_bytes));
    pcontroller->headlines()->push_back(param);

    return 0;
}

int callback_cfg_parameters_items(void *data, int colomns_count, char **colomn_data_pointer, char **colomn_name_pointer) {

    if (data == nullptr)
        return 0;

    if (colomns_count < 4) {
        //TODO to log transaction error
        return 0;
    }

    if ((colomn_data_pointer[0] == nullptr) || (colomn_data_pointer[1] == nullptr)) {
        //TODO to log error
        return 0;
    }

    int colomnInd = 0;	// Переменная для инкрементирования позиции в буфере
    QString name = colomn_data_pointer[colomnInd++];
    uint16_t addr = atoi(colomn_data_pointer[colomnInd++]);
    uint16_t ind = atoi(colomn_data_pointer[colomnInd++]);
    QString val = colomn_data_pointer[colomnInd++];

    auto pcontroller = static_cast<DcController_v2*>(data);
    QSharedPointer<DcParamCfg_v2_Headline> headline = pcontroller->getHeadlineForAddress(addr);

    DcParamCfg_v2 *pparam = new DcParamCfg_v2(pcontroller->index(), name, addr, ind, val, headline);
    if (!pcontroller->params_cfg()->add(pparam, true)) {
        delete pparam;
        pparam = nullptr;
        return 0;
    }

    return 0;
}

int callback_boards(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr) {
        //TODO to log transaction error
        return 0;
    }

    if (argc < 4) {
        //TODO to log transaction error
        return 0;
    }

    if ((argv[0] == nullptr) || (argv[1] == nullptr) || (argv[2] == nullptr)) {
        //TODO to log error
        return 0;
    }

    int boardid = atoi(argv[0]);
    int slot = atoi(argv[1]);
    QString type = argv[2];
    int inst = 0;
    if (argv[3] != nullptr) {
        inst = atoi(argv[3]);
    }

    QString properties;
    if (argc > 4 && argv[4] != nullptr) {
        properties = argv[4];
    }

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcBoard *pboard = new DcBoard(pcontroller->index(), boardid, slot, type, inst, properties);
    if (!pcontroller->boards()->add(pboard, true)) {
        delete pboard;
        pboard = nullptr;
        return 0;
    }

    return 0;
}

int callback_board_cfg_parameters(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr) {
        //TODO to log transaction error
        return 0;
    }

    if (argc < 8) {
        //TODO to log transaction error
        return 0;
    }

    if ((argv[0] == nullptr) || (argv[1] == nullptr) || (argv[2] == nullptr)) {
        //TODO to log error
        return 0;
    }
    int boardid = atoi(argv[0]);
    int addr = atoi(argv[1]);
    int param = atoi(argv[2]);
    QString name = argv[3];
    int dtype = atoi(argv[4]);
    uint8_t flags = 0;
    QString flagstr;
    //if (argv[5] != nullptr)
    //	flagstr = argv[5];
    //if (flagstr.contains("w"))
    //	flags = 3;
    //else if (flagstr.contains("r"))
    //	flags = 1;

    if (argv[5] != nullptr)
        flagstr = argv[5];

    //// Старое обозначение флагов, теперь мы используем как в DepParamSystem
    //if (flagstr.contains("w"))				// Проверяем, какие флаги допустимы для этого параметра, ставим биты
    //	flags = param_flags_e::WRITE;
    //if (flagstr.contains("r"))
    //	flags |= param_flags_e::READ;

    flags = getAttributeIntFromString(flagstr);	// Преобразуем строку атрибутов в число

    QString value = argv[6];
    QString properties = argv[7];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcBoard *pboard = pcontroller->boards()->getById(boardid);
    if (pboard == nullptr) {
        // !!! В файле базы данных остаются записи, которые не используются.
        // !!! Потенциально возможна ситуация при которой не получится вставить новую запись из-за наличия записи с таким же 'primary key'
        return 0;
    }

    DcParamCfgBoard *pparam = new DcParamCfgBoard(pcontroller->index(), boardid, addr, param, name, (DefParamDataType)dtype, flags, value, properties);
    if (!pboard->params()->add(pparam, true)) {
        delete pparam;
        pparam = nullptr;
        return 0;
    }
    return 0;
}

int callback_signals_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 7)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr) || (argv[2] == nullptr) || (argv[4] == nullptr) || (argv[5] == nullptr))
        return 0;

    int signalid = atoi(argv[0]);
    int internalid = atoi(argv[1]);
    int direction = atoi(argv[2]);
    int type = atoi(argv[3]);
    int subtype = atoi(argv[4]);
    QString name = argv[5];
    QString properties = argv[6];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcSignal *psignal = new DcSignal(pcontroller->index(), signalid, internalid, (DefSignalDirection)direction, (DefSignalType)type, (DefSignalSubType)subtype, name, properties);
    if (!pcontroller->addSignal(psignal, true)) {
        delete psignal;
        psignal = nullptr;
        return 0;
    }
    return 0;
}

int callback_algs_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 4)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr))
        return 0;

    int algid = atoi(argv[0]);
    int position = atoi(argv[1]);
    QString name = argv[2];
    QString properties = argv[3];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcAlgInternal *palg = new DcAlgInternal(pcontroller->index(), algid, position, name, properties);
    if (!pcontroller->algs_internal()->add(palg, true)) {
        delete palg;
        palg = nullptr;
        return 0;
    }
    return 0;
}

int callback_alg_io_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 5)
        return 0;
    if ((argv[0] == nullptr) || (argv[1] == nullptr) || (argv[2] == nullptr) || (argv[3] == nullptr))
        return 0;

    int ioid = atoi(argv[0]);
    int algid = atoi(argv[1]);
    int algpin = atoi(argv[2]);
    DefIoDirection direction = (DefIoDirection)atoi(argv[3]);
    QString name = argv[4];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcAlgInternal *palg = pcontroller->algs_internal()->getById(algid);
    if (palg == nullptr) {
        return 0;
    }

    DcAlgIOInternal *algio = new DcAlgIOInternal(pcontroller->index(), ioid, algid, algpin, direction, name);
    if (!palg->ios()->add(algio, true)) {
        delete palg;
        palg = nullptr;
        return 0;
    }
    return 0;
}

int callback_algs_cfc_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 4)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr) || (argv[2] == nullptr))
        return 0;

    int algid = atoi(argv[0]);
    int position = atoi(argv[1]);
    QString name = argv[2];
    QString properties = argv[3];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcAlgCfc *palg = new DcAlgCfc(pcontroller->index(), algid, position, name, properties);
    if (!pcontroller->algs_cfc()->add(palg, true)) {
        delete palg;
        palg = nullptr;
        return 0;
    }
    return 0;
}

int callback_alg_cfc_io_controller(void *data, int argc, char **argv, char **azColName) {
    if (data == nullptr)
        return 0;

    if (argc < 5)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr) || (argv[2] == nullptr) || (argv[3] == nullptr))
        return 0;

    int ioid = atoi(argv[0]);
    int algid = atoi(argv[1]);
    int algpin = atoi(argv[2]);
    DefIoDirection direction = (DefIoDirection)atoi(argv[3]);
    QString name = argv[4];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcAlgCfc *palg = pcontroller->algs_cfc()->getById(algid);
    if (palg == nullptr)
        return 0;

    DcAlgIOCfc *algio = new DcAlgIOCfc(pcontroller->index(), ioid, algid, algpin, direction, name);
    if (!palg->ios()->add(algio, true)) {
        delete palg;
        palg = nullptr;
        return 0;
    }
    return 0;
}

int callback_matrix_alg_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 3)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr))
        return 0;

    int srcid = atoi(argv[0]);
    int dstid = atoi(argv[1]);
    QString properties = argv[2];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcMatrixElementAlg *pelem = new DcMatrixElementAlg(pcontroller->index(), srcid, dstid, properties);
    if (!pcontroller->matrix_alg()->add(pelem, true)) {
        delete pelem;
        pelem = nullptr;
        return 0;
    }
    return 0;
}

int callback_matrix_alg_cfc_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 3)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr))
        return 0;

    int srcid = atoi(argv[0]);
    int dstid = atoi(argv[1]);
    QString properties = argv[2];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcMatrixElementAlgCfc *pelem = new DcMatrixElementAlgCfc(pcontroller->index(), srcid, dstid, properties);
    if (!pcontroller->matrix_cfc()->add(pelem, true)) {
        delete pelem;
        pelem = nullptr;
        return 0;
    }
    return 0;
}

int callback_matrix_signals_controller(void *data, int argc, char **argv, char **azColName) {

    if (data == nullptr)
        return 0;

    if (argc < 3)
        return 0;

    if ((argv[0] == nullptr) || (argv[1] == nullptr))
        return 0;

    int srcid = atoi(argv[0]);
    int dstid = atoi(argv[1]);
    QString properties = argv[2];

    auto pcontroller = static_cast<DcController_v2*>(data);
    DcMatrixElementSignal *pelem = new DcMatrixElementSignal(pcontroller->index(), srcid, dstid, properties);
    if (!pcontroller->matrix_signals()->add(pelem, true)) {
        delete pelem;
        pelem = nullptr;
        return 0;
    }
    return 0;
}

void workerThread()
{
    while (gDbManager.isActive()) {
        if (gDbManager.testEvent())
            gDbManager.stepTransaction();
    }
}

} // namespace

DcDbManager& DcDbManager::getInstance()
{
    static DcDbManager    instance;
    return instance;
}

struct DcDbManager::StorageItem
{
    sqlite3* db = nullptr;
    QString filePath;
    DcController_v2 *device = nullptr;
};

DcController_v2 *DcDbManager::load(int32_t id, const QString &filePath, const QString &name)
{
    if (filePath.isEmpty())
        return nullptr;

    StorageItem item;
    item.filePath = filePath;
    int rc = sqlite3_open_v2(filePath.toStdString().c_str(), &item.db, SQLITE_OPEN_READWRITE, nullptr);
    if (rc != SQLITE_OK) {
        makeError(sqlite3_errmsg(item.db), item);
        sqlite3_close(item.db);
        return nullptr;
    }

    auto deleter = [=](DcController_v2* controller) {
        delete controller;
        sqlite3_close(item.db);
    };

    std::unique_ptr<DcController_v2, decltype(deleter)> device(new DcController_v2(id, filePath, name), deleter);

    if (!exec(item, SELECT_SETTINGS, callback_settings_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_CFG_HEADLINE, callback_cfg_parameters_headline, device.get()))
        return nullptr;
    if (!exec(item, SELECT_CFG_ITEMS, callback_cfg_parameters_items, device.get()))
        return nullptr;
    if (!exec(item, SELECT_BOARDS, callback_boards, device.get()))
        return nullptr;
    if (!exec(item, SELECT_BOARD_CFG_PARAMETERS, callback_board_cfg_parameters, device.get()))
        return nullptr;
    if (!exec(item, SELECT_SIGNALS, callback_signals_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_ALGS, callback_algs_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_ALG_IO, callback_alg_io_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_ALGS_CFC, callback_algs_cfc_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_ALG_CFC_IO, callback_alg_cfc_io_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_MATRIX_ALG, callback_matrix_alg_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_MATRIX_ALG_CFC, callback_matrix_alg_cfc_controller, device.get()))
        return nullptr;
    if (!exec(item, SELECT_MATRIX_SIGNALS, callback_matrix_signals_controller, device.get()))
        return nullptr;

    auto config = device.release();
    item.device = config;
    m_connections.emplace(id, item);
    return config;
}

bool DcDbManager::execute(int32_t uid, const QString &transaction)
{
    if (transaction.isEmpty())
		return false;

	{
		std::unique_lock ul(m_queueMutex);
		m_transactions.push(std::make_pair(uid, transaction));
		m_queueCheck.notify_one();
    }
	
    // gDataManager.projectDesc()->update();
	return true;
}

bool DcDbManager::beginTransaction(int32_t uid)
{
    return execute(uid, "BEGIN TRANSACTION");
}

bool DcDbManager::endTransaction(int32_t uid)
{
    return execute(uid, "END TRANSACTION");
}

void DcDbManager::close(int32_t uid)
{
    auto it = m_connections.find(uid);
    if (it == m_connections.end())
        return;

    sqlite3_close(it->second.db);
    m_connections.erase(it);
}

void DcDbManager::closeAll()
{
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it->second.db == nullptr)
            continue;

        sqlite3_close(it->second.db);
    }

    m_connections.clear();
}

bool DcDbManager::isActive() const
{
    return m_isActive;
}

bool DcDbManager::testEvent()
{
	std::unique_lock ul(m_queueMutex);
	if (m_transactions.empty()) {
		m_queueCheck.wait(ul, [this]() { return !m_transactions.empty() || !isActive();	});
		if (!isActive()) {
			return false;
		}
	}

	return true;
}

bool DcDbManager::stepTransaction()
{
	std::pair<int32_t, QString> ppair;

	{
        std::unique_lock lock(m_queueMutex);
        if (m_transactions.empty())
            return false;

		ppair = m_transactions.front();
		m_transactions.pop();        
	}

    StorageItem item = connection(ppair.first);
    if (!item.db) {
        makeError(QString("Не найдено соединенеие с db (id = %1): \"%2\"").arg(ppair.first).arg(ppair.second), item);
        return false;
    }

    int rc = sqlite3_exec(item.db, ppair.second.toStdString().c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        makeError(QString("%1: \"%2\"").arg(sqlite3_errmsg(item.db), ppair.second), item);
		return false;
	}
	return true;
}

DcDbManager::DcDbManager()
    : m_isActive(true)
{
    m_workerThread = std::thread(workerThread);
    m_connections.clear();
}

DcDbManager::~DcDbManager()
{
    m_isActive = false;
    m_queueCheck.notify_one();
    m_workerThread.join();
    closeAll();
}

bool DcDbManager::exec(const StorageItem &item, const char *sql, int(*callback)(void*, int, char**, char**), void *data)
{
    if (!item.db) {
        makeError(QString("Не найдено соединенеие с db: \"%1\"").arg(sql), item);
        return false;
    }

    int rc = sqlite3_exec(item.db, sql, callback, data, 0);
    if (rc != SQLITE_OK) {
        makeError(QString("%1: \"%2\"").arg(sqlite3_errmsg(item.db), sql), item);
        return false;
    }

    return true;
}

DcDbManager::StorageItem DcDbManager::connection(int32_t connectionid)
{
    auto it = m_connections.find(connectionid);
    if (it == m_connections.end())
        return StorageItem{};

    return it->second;
}

void DcDbManager::makeError(const QString &errMsg, const StorageItem &item)
{
    emit error(errMsg, item.filePath, item.device);
}

