#pragma once

#include <qfiledevice.h>

#include <device_operations/DcIDeviceOperation.h>
#include <dpc/sybus/ParamPack.h>

class QTemporaryDir;

class DcReadConfigOperation : public DcIDeviceOperation 
{
	Q_OBJECT

public:
	DcReadConfigOperation(DcController *device, QObject *parent = nullptr);
	~DcReadConfigOperation();

	QList<QPair<DcParam*, DcBoard*>> failedParams() const;
	QStringList failedFiles() const;
	QHash<int, Dpc::Sybus::ParamPackPtr> specialParams() const;

protected:
	virtual bool before() override;
	virtual bool exec() override;
	virtual bool after() override;

private slots:
	void onChannelProgress(int state);

private:
	bool clearDeviceFolder();
	bool readParams();
	bool readSpecialParams();
	bool readFiles();
	bool readConfigArchive();
	bool unpackConfigArchive();
	bool configureSignalBindings();

	bool readParam(DcParam* param, DcBoard* board = nullptr);
	bool createFile(const QString &fileName, const QByteArray &data, QFlags<QFileDevice::Permission> permissions);

private:
	QTemporaryDir *m_tempDir;
	QList<QPair<DcParam*, DcBoard*>> m_failedParams;
	QStringList m_failedFiles;
	QStringList m_downloadedFiles;
	QString m_configArchiveFile;
	QString m_archiveDbFile;
	int m_filesCount;
	int m_currentFile;
	
	QHash<int, Dpc::Sybus::ParamPackPtr> m_specialParams;
};