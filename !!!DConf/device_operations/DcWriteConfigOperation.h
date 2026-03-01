#pragma once

#include <device_operations/DcIDeviceOperation.h>

class QTemporaryDir;

class DcWriteConfigOperation : public DcIDeviceOperation
{
	Q_OBJECT

public:
	DcWriteConfigOperation(DcController *device, QObject *parent = nullptr);
	~DcWriteConfigOperation();

	QString ipAddress() const;

public:
    static bool compileBindindgs(DcController *device);

protected:
	virtual bool before() override;
	virtual bool exec() override;

private slots:
	void onChannelProgress(int state);

private:
	bool complieCrossTables();
	bool packConfigArchive();
	bool setPrepareWorkMode();
	bool prepareRestartFlag();
	bool checkRestartFlag();
	bool uploadFiles();
	bool writeParams();
	bool readIpAddress();
	bool saveAndRestart();
	
	bool writeParam(DcParam* param, DcBoard* board = nullptr);

private:
	QTemporaryDir *m_tempDir;
	QString m_configArchiveFile;
	QList<DcBoard*> m_writtenBoards;
	int m_filesCount;
	int m_currentFile;
	QString m_ip;
};
