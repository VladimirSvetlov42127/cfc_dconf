#pragma once

#include <qobject.h>
#include <qelapsedtimer.h>

#include <data_model/dc_controller.h>

class DcIDeviceOperation : public QObject
{
	Q_OBJECT

public:
	enum State
	{
		NoErrorState,
		WarningState,
		ErrorState
	};

	DcIDeviceOperation(const QString &name, DcController *device, int stepsCount = 1,  QObject *parent = nullptr);
	virtual ~DcIDeviceOperation() {}

	State state() const;
	QStringList reportList() const;

signals:
	void progress(int value);
	void finished(int state);

	void infoMsg(const QString &msg, DcController *device);
	void warningMsg(const QString &msg, DcController *device);
	void errorMsg(const QString &msg, DcController *device);
	void debugMsg(const QString &msg, int level, DcController *device);

public slots:
	void start();

protected slots:
	void addReport(const QString &report, bool warning = true);
	void addInfo(const QString &msg);
	void addWarning(const QString &msg);
	void addError(const QString &msg);
	void addDebug(const QString &msg, int level = 0);
	void emitProgress(int currentStepValue);

	bool saveConfig(DcBoard* board = nullptr);
	bool restart(DcBoard* board = nullptr);

protected:
	DcController* device() const;
	ChannelPtr channel() const;

	void setCheckSoftIdEnabled(bool enabled);
	void abort(const QString &msg = QString());
	void setCurrentStep(int step, int total = 100);

	virtual bool before() { return true; }
	virtual bool exec() = 0;
	virtual bool after() { return true; }

	static QString configArchiveDevicePath();

private slots:
	void onChannelDebug(int level, const QString &msg);
	void onChannelInfo(const QString& msg);

private:
	void finish(DcIDeviceOperation::State state);
	bool checkSoftId();

private:
	QString m_name;
	DcController *m_device;
	QStringList m_reportsList;
	DcIDeviceOperation::State m_state;

	int m_stepsCount;
	int m_currentStep;
	int m_currentStepTotal;
	bool m_isCheckSoftIdEnabled;

	QElapsedTimer m_timer;
};