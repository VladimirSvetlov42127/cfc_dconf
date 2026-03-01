#include "updater.h"

#include <qdebug.h>
#include <qprocess.h>

namespace {
#ifdef QT_DEBUG
	const QString UPDATER_TOOL_PATH = "../3rdparty/maintenancetool/maintenancetool";
#else
	const QString UPDATER_TOOL_PATH = "../maintenancetool";
#endif	

	const QString ARG_CHECK_UPDATES = "check-updates";
	const QString ARG_START_UPDATES = "--start-updater";
}

DcUpdater::DcUpdater(QObject* parent) :
	QObject(parent),
	m_process(new QProcess(this))
{
	connect(m_process, qOverload<int, QProcess::ExitStatus >(&QProcess::finished), this, &DcUpdater::onProcessFinished);
	connect(m_process, &QProcess::errorOccurred, this, &DcUpdater::onProcessError);
	connect(m_process, &QProcess::readyReadStandardOutput, this, &DcUpdater::onReadyReadStandard);
	connect(m_process, &QProcess::readyReadStandardError, this, &DcUpdater::onReadyReadError);
}

DcUpdater::~DcUpdater()
{
}

void DcUpdater::checkUpdates()
{
	m_process->start(UPDATER_TOOL_PATH, { ARG_CHECK_UPDATES } );
}

bool DcUpdater::startUpdates()
{
	QProcess p;
	p.setProgram(UPDATER_TOOL_PATH);
	p.setArguments({ ARG_START_UPDATES });
	if (!p.startDetached()) {
		emit error(m_process->errorString());
		return false;
	}

	return true;
}

void DcUpdater::onProcessError()
{
	emit error(m_process->errorString());
}

void DcUpdater::onProcessFinished(int exitCode)
{
	if (QProcess::NormalExit != m_process->exitStatus()) {
		error(QString("process crash %1").arg(m_process->errorString()));
		return;
	}

	onReadyReadStandard();
	bool hasUpdates = true;
	if (m_data.isEmpty() || QString::fromUtf8(m_data).split("<updates>").size() < 2) {
		hasUpdates = false;
	}

	emit updatesAvailableState(hasUpdates);
}

void DcUpdater::onReadyReadStandard()
{
	m_data += m_process->readAllStandardOutput();
}

void DcUpdater::onReadyReadError()
{
}