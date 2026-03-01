#ifndef DCDBMANAGER_H
#define DCDBMANAGER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>

#include <qstring.h>
#include <QObject>

#include <db/sqlite/sqlite3.h>
#include <data_model/dc_controller.h>

#define gDbManager DcDbManager::getInstance()

// 1. forward declaration sqlite3
// 2. Наследовать DcDbManager от класса QObject и добавить сигнал с ошибкой

class DcController_v2;

class DcDbManager : public QObject
{
    Q_OBJECT
public:
    static DcDbManager& getInstance();

    DcController_v2* load(int32_t id, const QString &filePath, const QString &name);
    bool execute(int32_t uid, const QString &transaction);
    bool beginTransaction(int32_t uid);
    bool endTransaction(int32_t uid);
    void close(int32_t uid);
    void closeAll();

    bool isActive() const;
    bool testEvent();
    bool stepTransaction();

signals:
    void error(const QString &msg, const QString &filePath, DcController *device);

private:
    struct StorageItem;

    DcDbManager();
    ~DcDbManager();
    DcDbManager(DcDbManager const&) = delete;
    DcDbManager& operator= (DcDbManager const&) = delete;
    DcDbManager(DcDbManager&&) = delete;
    DcDbManager& operator= (DcDbManager&&) = delete;

    bool exec(const StorageItem &item, const char *sql, int(*callback)(void*, int, char**, char**), void *data);
    StorageItem connection(int32_t connectionid);
    void makeError(const QString &errMsg, const StorageItem &item);

private:
	std::queue<std::pair<int32_t, QString>> m_transactions;
    std::map<int32_t, StorageItem> m_connections;

	std::thread m_workerThread;
	std::mutex m_queueMutex;
    std::condition_variable m_queueCheck;

    bool m_isActive;
};

#endif // DCDBMANAGER_H
